#include <stdint.h>

#include "configure.h"
#include "board.h"
#include "dbgu_console.h"
#include "ddr.h"

#define LC_INCLUDE "lc-addrlabels.h"
#include "pt.h"


/**
 * Firmware main() function
 */
int main(int argc, char **argv)
{
#if defined(CONFIGURE_TRACE) && (CONFIGURE_TRACE_PORT == TRACE_PORT_DBGU)
	DBGU_ConsoleUseDBGU();
	DBGU_Configure(115200, BOARD_MCK);
#endif

	TRACE_DEBUG("%s %s", __TIME__, __DATE__);

#if (CONFIGURE_DDR2 != 0)
	ddr_configure(DDR_TYPE_MT47H64M16HR);
#endif

#if (CONFIGURE_DDR_TEST != 0)
	ddr_test();
#endif

    return 0;
}
