#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>


#include "configure.h"
#include "stdio.h"
#include "cmd.h"
#include "dbgu_console.h"


#define PRINT_TRACE 0
extern void toggle_led_count(uint8_t count);

unsigned char uart_rx_buffer[255];
unsigned char uart_rx_buffer_size;


const cmd_t gen_commands[] = {
	ADD_COMMAND(cmd_ping, CMD_PING),
	ADD_COMMAND(cmd_exit, CMD_EXIT),
};

int gen_commands_total = sizeof(gen_commands)/sizeof(gen_commands[0]);

/**
 * System statistics
 */
cmd_stat_t cmd_stat;


unsigned char cmd_shift_rx_buffer(void)
{
	uart_rx_buffer_size--;
	memmove(&uart_rx_buffer[0], &uart_rx_buffer[1], uart_rx_buffer_size);

	return 0;
}

static inline unsigned char find_synchronization(void);
static inline unsigned char find_synchronization(void)
{
	return 1;
}


static unsigned char calculate_checksum(unsigned char size)
{
	unsigned char sum = 0;
	unsigned char i;
	unsigned char *s = uart_rx_buffer;

	size = size - 1;
	for (i = 0;i < size;i++,s++)
		sum += *s;
	sum = ~sum + 1;

	return sum;
}

static unsigned char checksum(unsigned char size)
{
	unsigned char sum;
	unsigned char cs;

	sum = calculate_checksum(size);
	cs = uart_rx_buffer[size - 1];

	return (sum == cs);
}

/**
 * I am going to reset this variable after I get first command
 */
char cmd_first_command = 1;

unsigned char cmd_send_slave(unsigned char size)
{
	unsigned char cs;
	
	size += RAW_CMD_SIZE;

	// field size is at least 1 - one byte of checksum
	uart_rx_buffer[PAYLOAD_SIZE_OFFSET] = size - HEADER_SIZE;

	if (!cmd_first_command)      // in all responses but very first
	{                        // MSB is set
		uart_rx_buffer[COMMAND_ID_OFFSET] |= 0x80;
	}


	cs = calculate_checksum(size);
	uart_rx_buffer[size - 1] = cs;
	uart_rx_buffer_size = size;

#if INTRODUCE_DELAY
	{
		uint32_t tick = tc_sys_tick;
		tick += 1000;
		while (tc_sys_tick != tick) ;
	}
#endif
	DBGU_PutBuffer(uart_rx_buffer, size);

	return 0;
}

/**
 * Find command with specified command id
 * Return reference to command if found, 0 if fails
 */
const cmd_t *cmd_find_command(unsigned char command_id)
{
	const cmd_t *cmd;
	int i;

	for (i = 0; i < gen_commands_total; i++)
	{
		cmd = &gen_commands[i];
		if (cmd->command_id == command_id)
		{
			return cmd;
		}
	}

	return 0;
}

unsigned char process_command()
{
	unsigned char ret = 0;
	unsigned char res;
	const cmd_t *cmd;
	unsigned char expected_size, command_size;
	unsigned char command_id;

	cmd_stat.process_command++;

	while (1)
	{

		// check that the command is well formed
		if (uart_rx_buffer_size <= PAYLOAD_OFFSET) // I did not get all bytes yet
			break;

#if (PRINT_TRACE > 1)
		cmd_printf("Got data: size = %d (0x%02X 0x%02X 0x%02X)\r\n", uart_rx_buffer_size, uart_rx_buffer[0], uart_rx_buffer[1], uart_rx_buffer[2]);
#endif
		res = find_synchronization();
		if (!res)
		{
			cmd_shift_rx_buffer();
			break;
		}

		command_id = cmd_get_id(uart_rx_buffer);
		cmd = cmd_find_command(command_id);
		if (cmd == 0)
		{
			cmd_stat.process_command_bad_id++;
#if (PRINT_TRACE > 0)
			cmd_printf("ID: size = %d (0x%02X 0x%02X 0x%02X) \r\n", uart_rx_buffer_size, uart_rx_buffer[0], uart_rx_buffer[1], uart_rx_buffer[2]);
#endif
			cmd_shift_rx_buffer();
			continue;
		}

		expected_size = cmd->size;
		command_size = uart_rx_buffer[PAYLOAD_SIZE_OFFSET] + PAYLOAD_OFFSET; // payload size + 4 bytes of header
#if (PRINT_TRACE > 1)
		cmd_printf("Cmd found: %x, cmd size = %d, exp. size = %d\r\n", command_id, command_size, expected_size);
#endif
		if (expected_size != command_size)
		{
			cmd_stat.process_command_bad_size++;
#if (PRINT_TRACE > 0)
			cmd_printf("Szie: size = %d in.of %d (0x%02X 0x%02X 0x%02X) \r\n", uart_rx_buffer_size, expected_size, uart_rx_buffer[0], uart_rx_buffer[1], uart_rx_buffer[2]);
#endif
			cmd_shift_rx_buffer();
			continue;
		}

		if (uart_rx_buffer_size < expected_size)
		{
			cmd_stat.process_command_not_full++;
			break;
		}

		res = checksum(expected_size);
		if (!res)
		{
#if (PRINT_TRACE > 0)
			cmd_printf("CS: size = %d (%02X %02X %02X %02X %02X %02X %02X) \r\n", uart_rx_buffer_size, uart_rx_buffer[0], uart_rx_buffer[1], uart_rx_buffer[2],
					uart_rx_buffer[3], uart_rx_buffer[4], uart_rx_buffer[5], uart_rx_buffer[6]);
#endif
			cmd_stat.process_command_bad_cs++;
			cmd_shift_rx_buffer();
			continue;
		}

		cmd_stat.process_command_handler++;
		// This is a legal command
		cmd->handler(expected_size - RAW_CMD_SIZE );
		cmd_stat.process_command_handler_done++;

		if (expected_size < uart_rx_buffer_size)
		{
#if (PRINT_TRACE > 0)
			cmd_printf("NotEmpty: size = %d (0x%02X 0x%02X 0x%02X) \r\n", uart_rx_buffer_size, uart_rx_buffer[0], uart_rx_buffer[1], uart_rx_buffer[2]);
#endif
			cmd_stat.process_command_buffer_ne++;
		}
		uart_rx_buffer_size = 0;
		cmd_first_command = 0;
		ret = 1;

		break;
	}

	return ret;
}


CMD_DECLARE_FUNCTION(cmd_ping)
{
	TRACE_DEBUG("PING");
	cmd_send_slave(size);

	return 0;
}

int cmd_exit_main_loop_flag = 0;
CMD_DECLARE_FUNCTION(cmd_exit)
{
	TRACE_DEBUG("EXIT");
	cmd_send_slave(size);
	cmd_exit_main_loop_flag = 1;
	return 0;
}

int cmd_printf(const char *fmt,  ... )
{
    va_list ap;
    int ret;
    char buf[64];
    va_start(ap, fmt);
    ret = vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    if ((ret > 0) && (ret < sizeof(buf)))
    {
    	DBGU_PutBuffer((unsigned char*)buf, ret);
    }
    return ret;
}

void cmd_mem_printf(const char *fmt,  ... )
{
    va_list ap;
    va_start(ap, fmt);
    vsnprintf((char*)0x308000, 80, fmt, ap);
    va_end(ap);
}
