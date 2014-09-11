#include <stdint.h>


#include "configure.h"
#include "ddr.h"

/**
 * Firmware main() function
 */
int main(int argc, char **argv)
{
#if (CONFIGURE_DDR2 != 0)
	ddr_configure(DDR_TYPE_MT47H64M16HR);
#endif

#if (CONFIGURE_DDR_TEST != 0)
	ddr_test();
#endif

    return 0;
}
