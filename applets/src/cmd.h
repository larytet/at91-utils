#ifndef _PROCESS_CMD_H_INCLUDED_
#define _PROCESS_CMD_H_INCLUDED_


typedef unsigned char  (*cmd_uart_tx_putbuf_t)(const unsigned char *s, unsigned char len);

extern char cmd_first_command;

#define PAYLOAD_OFFSET       2
#define PAYLOAD_SIZE_OFFSET  1
#define COMMAND_ID_OFFSET    0
#define HEADER_SIZE          PAYLOAD_OFFSET
#define CS_SIZE              1
#define RAW_CMD_SIZE         (HEADER_SIZE + CS_SIZE)

#define CMD_FULL_SIZE(payload_size)   (RAW_CMD_SIZE + payload_size)

/**
 * Read/write memory location commands can contain
 * this values
 */
#define READ_ACCESS_8   (0x00 | 0x00)
#define WRITE_ACCESS_8  (0x00 | 0x01)
#define READ_ACCESS_16  (0x02 | 0x00)
#define WRITE_ACCESS_16 (0x02 | 0x01)
#define READ_ACCESS_32  (0x06 | 0x00)
#define WRITE_ACCESS_32 (0x06 | 0x01)

#define IS_READ_ACCESS(type) (((type) & 0x01) == 0)
#define IS_ACCESS_8(type) (((type) & 0x02) == 0)

#define CMD_PRINT_BUFF_SIZE 84

typedef struct __attribute__ ((__packed__))
{
	
	unsigned char cmd;
	unsigned char size;

} cmd_hdr_t;

/**
 * The size of the whole command string including 1 byte of checksum
 * This the same as size of the payload + header (2) + checksum (1)
 * This is the same  as size of the payload + RAW_CMD_SIZE (3)
 */
#define CMD_EXP_SIZE(func)         (sizeof(func ## _rq_t) + 1)

#define ADD_COMMAND(func, cmd_id)  { CMD_EXP_SIZE(func), func, 0, cmd_id }


/**
 * This is common commands
 */
enum
{
	CMD_ERROR									= 0x00, // 0x00 - Reserved
	CMD_PING									= 0x03, // 0x03 - Ping
	CMD_EXIT									,       // 0x04 - Exit main loop
	CMD_LAST_COMMON 							= 0x2B, // 0x2B - Last common command
};

/**
 * This is how cmd looks like in the array of supported commands
 */
typedef struct
{
	/**
	 * number of bytes in the message
	 */
	unsigned char size;

	/**
	 * callback which processes the command and sends response
	 */
	unsigned char (*handler)(unsigned char size);

	/**
	 * number of times the command called
	 */
	int calls;

	/**
	 * command identifier
	 */
	int command_id;
} cmd_t;

/**
 * Size of the array which contains all supported commands
 * Usually something like (sizeof(commands)/sizeof(commands[0])) will do
 * The variable should be declared in the cmd.c
 */
extern int commands_total;

/**
 * Array of commands handlers
 */
extern const cmd_t commands[];

/**
 * This macro will be helpful if I decide to modify prototype of
 * the call-backs in the future
 */
#define CMD_DECLARE_FUNCTION(name) unsigned char name(unsigned char size)



/**
 * Complete FSM which handles incoming commands
 * Returns non-zero (size of the command) if a
 * complete command was handled in this call
 */
extern unsigned char process_command(void);

/**
 * Get command handler by command identifier
 */
const cmd_t *cmd_find_command(unsigned char command_id);

/**
 * Return command identifier from the packet
 */
static inline unsigned char cmd_get_id(unsigned char *buffer)
{
	return buffer[COMMAND_ID_OFFSET];
}

/**
 * Return command identifier from the packet
 */
static inline unsigned char cmd_get_size(unsigned char *buffer)
{
	return buffer[PAYLOAD_SIZE_OFFSET];
}

/**
 * Send response (from peripheral card) of the specified size.
 * The data is in the rx_buffer
 */
unsigned char cmd_send_slave(unsigned char size);

/**
 * Debug counters
 */
typedef struct __attribute__ ((__packed__))
{
	uint16_t uart_rx_irq;
	uint16_t uart_rx_irq_add;
	uint16_t uart_rx_of;
} uart_stat_t;

/**
 * Debug counters (current version 3a11)
 */
typedef struct __attribute__ ((__packed__))
{
	uint16_t mdio_write;
	uint16_t mdio_read;
	uint16_t mdio_cmd;
	uint16_t reset;
	uint16_t turnaround_bit;
} mdio_stat_t;

/**
 * Debug counters (version 3a10)
 */
typedef struct __attribute__ ((__packed__))
{
	uint16_t mdio_write;
	uint16_t mdio_read;
	uint16_t mdio_cmd;
	uint16_t reset;
	uint16_t turnaround_bit;
} mdio_stat_3a11_t;

/**
 * Debug counters (version 3a10)
 */
typedef struct __attribute__ ((__packed__))
{
	uint16_t mdio_write;
	uint16_t mdio_read;
	uint16_t mdio_cmd;
} mdio_stat_3a10_t;

/**
 * Debug counters
 */
typedef struct __attribute__ ((__packed__))
{
	uint16_t i2c_rd_total;
	uint16_t i2c_wr_total;
	uint16_t i2c_rd_noack_1;
	uint16_t i2c_rd_noack_2;
	uint16_t i2c_rd_noack_3;
	uint16_t i2c_wr_noack_1;
	uint16_t i2c_wr_noack_2;
	uint16_t i2c_wr_noack_3;
	uint16_t waitforcomplete;
	uint16_t waitforcomplete1;
	uint16_t waitforcomplete2;
	uint16_t waitforcompleterd;
	uint16_t waitforcompletesta;
	uint16_t hw_ack;
	uint16_t hw_nack;
	uint8_t  twsr[16];
	uint16_t twsr_idx;
	uint16_t rd_cmd_ack;
	uint16_t wr_cmd_ack;
	uint16_t arblostcleared;
	uint16_t arblostnotcleared;
} i2c_stat_t;

/**
 * Debug counters
 */
typedef struct __attribute__ ((__packed__))
{
	uint16_t i2c_rd_total;
	uint16_t i2c_wr_total;
	uint16_t i2c_rd_noack_1;
	uint16_t i2c_rd_noack_2;
	uint16_t i2c_rd_noack_3;
	uint16_t i2c_wr_noack_1;
	uint16_t i2c_wr_noack_2;
	uint16_t i2c_wr_noack_3;
	uint16_t waitforcomplete;
	uint16_t waitforcomplete1;
	uint16_t waitforcomplete2;
	uint16_t waitforcompleterd;
	uint16_t waitforcompletesta;
	uint16_t hw_ack;
	uint16_t hw_nack;
	uint8_t  twsr[16];
	uint16_t twsr_idx;
	uint16_t rd_cmd_ack;
	uint16_t wr_cmd_ack;
	uint16_t arblostcleared;
	uint16_t arblostnotcleared;
} i2c_stat_3a13_t;

/**
 * Debug counters
 */
typedef struct __attribute__ ((__packed__))
{
	uint16_t i2c_rd_total;
	uint16_t i2c_wr_total;
	uint16_t i2c_rd_noack_1;
	uint16_t i2c_rd_noack_2;
	uint16_t i2c_rd_noack_3;
	uint16_t i2c_wr_noack_1;
	uint16_t i2c_wr_noack_2;
	uint16_t i2c_wr_noack_3;
#if 1
	uint16_t waitforcomplete;
	uint16_t waitforcomplete1;
	uint16_t waitforcomplete2;
	uint16_t hw_ack;
	uint16_t hw_nack;
	uint8_t  twsr[16];
	uint16_t twsr_idx;
	uint16_t rd_cmd_ack;
	uint16_t wr_cmd_ack;
#endif
} i2c_stat_3a12_t;

/**
 * Debug counters
 */
typedef struct __attribute__ ((__packed__))
{
	uint16_t i2c_noack_1;
	uint16_t i2c_noack_2;
	uint16_t i2c_noack_3;
} i2c_stat_3a11_t;

/**
 * Debug counters
 */
typedef struct __attribute__ ((__packed__))
{
	uint16_t process_command;
	uint16_t process_command_bad_id;
	uint16_t process_command_bad_size;
	uint16_t process_command_bad_cs;
	uint16_t process_command_handler;
	uint16_t process_command_handler_done;
	uint16_t process_command_am_read;
	uint16_t process_command_am_write;
	uint16_t process_command_not_full;
	uint16_t process_command_buffer_ne;
} cmd_stat_t;

/**
 * Debug counters (version 3a13)
 */
typedef struct __attribute__ ((__packed__))
{
	uint16_t process_command;
	uint16_t process_command_bad_id;
	uint16_t process_command_bad_size;
	uint16_t process_command_bad_cs;
	uint16_t process_command_handler;
	uint16_t process_command_handler_done;
	uint16_t process_command_am_read;
	uint16_t process_command_am_write;
	uint16_t process_command_not_full;
	uint16_t process_command_buffer_ne;
} cmd_stat_3a13_t;

/**
 * Debug counters (version 3a12)
 */
typedef struct __attribute__ ((__packed__))
{
	uint16_t process_command;
	uint16_t process_command_bad_id;
	uint16_t process_command_bad_size;
	uint16_t process_command_bad_cs;
	uint16_t process_command_handler;
	uint16_t process_command_handler_done;
	uint16_t process_command_am_read;
	uint16_t process_command_am_write;
	uint16_t process_command_not_full;
} cmd_stat_3a12_t;

/**
 * System statistics. It is placed here by accident
 * File sys.h would be a better place but it does not
 * exist
 */
extern cmd_stat_t cmd_stat;

extern unsigned char uart_rx_buffer[255];
extern unsigned char uart_rx_buffer_size;

static inline void uart_rx_buffer_add(unsigned char c)
{
	uart_rx_buffer[uart_rx_buffer_size] = c;
	uart_rx_buffer_size++;
	if (uart_rx_buffer_size >= sizeof(uart_rx_buffer))
	{
		uart_rx_buffer_size= 0;
	}
}

extern unsigned char cmd_shift_rx_buffer(void);
extern char watchdog_enabled;
extern const unsigned char FW_VERSION[3];


typedef struct __attribute__ ((__packed__))
{
	cmd_hdr_t hdr;

}cmd_ping_rq_t;

typedef struct __attribute__ ((__packed__))
{
	cmd_hdr_t hdr;

}cmd_ping_rs_t;


typedef struct __attribute__ ((__packed__))
{
	cmd_hdr_t hdr;

}cmd_exit_rq_t;

typedef struct __attribute__ ((__packed__))
{
	cmd_hdr_t hdr;

} cmd_exit_rs_t;


extern CMD_DECLARE_FUNCTION (cmd_ping);
extern CMD_DECLARE_FUNCTION (cmd_exit);

extern int cmd_exit_main_loop_flag;

#if (CONFIGURE_TRACE != 0)
#define TRACE_DEBUG(fmt, ... )      cmd_printf(fmt,  ##__VA_ARGS__)
#else
#define TRACE_DEBUG(fmt, ... )      { }
#endif

extern int cmd_printf(const char *fmt, ...);

extern void cmd_mem_printf(const char *fmt, ...);

/**
 * Debug - mark location in the internal RAM and get out of the function
 */
#define CMD_MARK_AND_EXIT(res)                                                   \
{                                                                              \
    *((volatile uint32_t*)0x308000) = 0xE0A111E1;                            \
    return res;                                                                    \
}

#endif // _PROCESS_CMD_H_INCLUDED_
