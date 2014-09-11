#include <stdint.h>

/**
 * Firmware main() function
 */
int main(int argc, char **argv)
{
	*(volatile uint32_t*)0x308000 = 0x1f4e5601;
    return 0;
}
