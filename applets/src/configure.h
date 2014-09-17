#ifndef _CONFGIGURE
#define _CONFGIGURE


#define BOARD_SAMA5D3X 1

#define CONFIGURE_TRACE                  1
#define TRACE_PORT_DBGU                  1
#define TRACE_PORT_UART0                 2
#define CONFIGURE_TRACE_PORT             TRACE_PORT_DBGU

// Build configuration
#define BOARD BOARD_SAMA5D3X
#define CONFIGURE_DDR2                  1
#define CONFIGURE_DDR_TEST              1
#define CONFIGURE_DDR_TEST_SIZE   (10*1024)

/**
 * Enable command loop
 */
#define CONFIGURE_CMD                  1

/**
 * Enable command 'ping'
 */
#define CONFIGURE_CMD_PING             1

/**
 * Enable debug statistics
 */
#define CONFIGURE_CMD_STAT             0

/** Frequency of the board main oscillator */
#define BOARD_MAINOSC           12000000



#endif // _CONFGIGURE
