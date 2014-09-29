#include <stdint.h>
#include <stdlib.h>

#include "configure.h"
#include "board.h"
#include "board_sama5d3x.h"
#include "dbgu_console.h"
#include "ddr.h"
#include "cmd.h"

#define LC_INCLUDE "lc-addrlabels.h"
#include "pt.h"


#if (CONFIGURE_MAIN_LOOP != 0) && (CONFIGURE_CMD != 0)
static unsigned char pt_uart_rx(struct pt *pt)
{
	PT_BEGIN(pt);

	while (1)
	{
		unsigned char c;
		uint32_t res = (unsigned char)DBGU_GetChar(&c);
		if (res)
		{
			uart_rx_buffer_add(c);
		}
		else
		{
			PT_YIELD(pt);
		}
	}

	PT_END(pt);
}
#endif

#if (CONFIGURE_MAIN_LOOP != 0) && (CONFIGURE_CMD != 0)
static unsigned char pt_uart_tx(struct pt *pt)
{
	PT_BEGIN(pt);

	while (1)
	{
		PT_YIELD_UNTIL(pt, uart_rx_buffer_size != 0);
		process_command();
	}

	PT_END(pt);
}
#endif

#if (CONFIGURE_MAIN_LOOP != 0) && (CONFIGURE_CMD != 0)
static struct pt pt_uart_rx_state;
static struct pt pt_uart_tx_state;
#endif

static void main_configure_cs(void)
{

#if (CONFIGURE_CS != 0) && (CONFIGURE_CS_1 != 0)
	PIOE->PIO_PDR = PIO_PDR_P27;
	SMC->SMC_CS_NUMBER[1].SMC_SETUP = 0x00080008;
	SMC->SMC_CS_NUMBER[1].SMC_PULSE = 0x18081808;
	SMC->SMC_CS_NUMBER[1].SMC_CYCLE = 0x00180018;
	SMC->SMC_CS_NUMBER[1].SMC_MODE = SMC_MODE_WRITE_MODE | SMC_MODE_READ_MODE;
#endif

#if (CONFIGURE_CS != 0) && (CONFIGURE_CS_2 != 0)
	SMC->SMC_CS_NUMBER[2].SMC_SETUP = 0x00080008;
	SMC->SMC_CS_NUMBER[2].SMC_PULSE = 0x18081808;
	SMC->SMC_CS_NUMBER[2].SMC_CYCLE = 0x00180018;
	SMC->SMC_CS_NUMBER[1].SMC_MODE = SMC_MODE_WRITE_MODE | SMC_MODE_READ_MODE;
#endif
}

/**
 * Firmware main() function
 */
int main(int argc, char **argv)
{

#if defined(CONFIGURE_TRACE) && (CONFIGURE_TRACE_PORT == TRACE_PORT_DBGU)
	DBGU_ConsoleUseDBGU();
	DBGU_Configure(115200, 47923200); // BOARD_MAINOSC/4); // 47923200? 166000000, 165000000 BOARD_MCK
#endif

	TRACE_DEBUG("%s %s", __TIME__, __DATE__);

	main_configure_cs();

#if (CONFIGURE_DDR2 != 0)
	ddr_configure(DDR_TYPE_MT47H64M16HR);
#endif

#if (CONFIGURE_DDR_TEST != 0)
	ddr_test();
#endif

#if (CONFIGURE_MAIN_LOOP != 0)
#if (CONFIGURE_CMD != 0)
	PT_INIT(&pt_uart_rx_state);
	PT_INIT(&pt_uart_tx_state);
#endif
#endif

	TRACE_DEBUG("Main loop");

#if (CONFIGURE_MAIN_LOOP != 0)
	while (cmd_exit_main_loop_flag == 0)
	{
		pt_uart_rx(&pt_uart_rx_state);
		pt_uart_tx(&pt_uart_tx_state);
	}
#endif


	return 0;
}



