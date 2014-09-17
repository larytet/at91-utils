#ifndef _BOARD_
#define _BOARD_

#define BOARD BOARD_SAMA5D3X

#if (BOARD == BOARD_SAMA5D3X)
#include "board_sama5d3x.h"
#endif

#include "pio.h"
#include "pmc.h"

/** Master clock frequency (when using board_lowlevel.c) */
#define BOARD_MCK                ((unsigned long)((BOARD_MAINOSC / 3 / 2) * 66 ))



/** List of all DBGU pin definitions. */

/** DBGU Monitor IO pin (detect any DBGU operation). */
#define PIN_DBGU_MON {PIO_PA9A_DRXD, PIOA, ID_PIOA, PIO_INPUT, PIO_IT_RISE_EDGE}
/** DBGU pin definition. */
#define PINS_DBGU   {(1 << 30) | (1 << 31), PIOB, ID_PIOB, PIO_PERIPH_A, PIO_DEFAULT}

/** List of all USART pin definitions. */

#define PIO_TXD0   (1 << 18)
#define PIO_RXD0   (1 << 17)
#define PIO_RTS0   (1 << 16)
#define PIO_CTS0   (1 << 15)
#define PIO_SCK0   (1 << 14)

/** USART0 TXD pin definition. */
#define PIN_USART0_TXD  {PIO_TXD0, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
/** USART0 RXD pin definition. */
#define PIN_USART0_RXD  {PIO_RXD0, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
/** USART0 RTS pin definition. */
#define PIN_USART0_RTS  {PIO_RTS0, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
/** USART0 CTS pin definition. */
#define PIN_USART0_CTS  {PIO_CTS0, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
/** USART0 SCK pin definition. */
#define PIN_USART0_SCK  {PIO_SCK0, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}

/** USART1 TXD pin definition. */
#define PIN_USART1_TXD  {PIO_PA5A_TXD1, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
/** USART1 RXD pin definition. */
#define PIN_USART1_RXD  {PIO_PA6A_RXD1, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
/** USART1 RTS pin definition. */
#define PIN_USART1_RTS  {PIO_PC27C_RTS1, PIOC, ID_PIOC, PIO_PERIPH_C, PIO_DEFAULT}
/** USART1 CTS pin definition. */
#define PIN_USART1_CTS  {PIO_PC28C_CTS1, PIOC, ID_PIOC, PIO_PERIPH_C, PIO_DEFAULT}
/** USART1 SCK pin definition. */
#define PIN_USART1_SCK  {PIO_PC29C_SCK1, PIOC, ID_PIOC, PIO_PERIPH_C, PIO_DEFAULT}

/** USART2 TXD pin definition. */
#define PIN_USART2_TXD  {PIO_PA7A_TXD2, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
/** USART2 RXD pin definition. */
#define PIN_USART2_RXD  {PIO_PA8A_RXD2, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
/** USART2 RTS pin definition. */
#define PIN_USART2_RTS  {PIO_PB0B_RTS2, PIOB, ID_PIOB, PIO_PERIPH_B, PIO_DEFAULT}
/** USART2 CTS pin definition. */
#define PIN_USART2_CTS  {PIO_PB1B_CTS2, PIOB, ID_PIOB, PIO_PERIPH_B, PIO_DEFAULT}
/** USART2 SCK pin definition. */
#define PIN_USART2_SCK  {PIO_PB2B_SCK2, PIOB, ID_PIOB, PIO_PERIPH_B, PIO_DEFAULT}


/** List of all TWI pin definitions. */

/** TWI0 data pin */
#define PIN_TWI_TWD0   {(1 << 30), PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
/** TWI0 clock pin */
#define PIN_TWI_TWCK0  {(1 << 31), PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
/** TWI0 pins */
#define PINS_TWI0      PIN_TWI_TWD0, PIN_TWI_TWCK0

/** List of all SPI pin definitions. */

/** SPI0 MISO pin definition. */
#define PIN_SPI0_MISO     {(1 << 10), PIOD, ID_PIOD, PIO_PERIPH_A, PIO_DEFAULT}
/** SPI0 MOSI pin definition. */
#define PIN_SPI0_MOSI     {(1 << 11), PIOD, ID_PIOD, PIO_PERIPH_A, PIO_DEFAULT}
/** SPI0 SPCK pin definition. */
#define PIN_SPI0_SPCK     {(1 << 12), PIOD, ID_PIOD, PIO_PERIPH_A, PIO_DEFAULT}
/** SPI0 chip select pin definition. */
#define PIN_SPI0_NPCS0    {(1 << 13), PIOD, ID_PIOD, PIO_PERIPH_A, PIO_DEFAULT}
/** List of SPI0 pin definitions (MISO, MOSI & SPCK). */
#define PINS_SPI0         PIN_SPI0_MISO, PIN_SPI0_MOSI, PIN_SPI0_SPCK

/** SPI1 MISO pin definition. */
#define PIN_SPI1_MISO     {PIO_PA21B_SPI1_MISO, PIOA, ID_PIOA, PIO_PERIPH_B, PIO_DEFAULT}
/** SPI1 MOSI pin definition. */
#define PIN_SPI1_MOSI     {PIO_PA22B_SPI1_MOSI, PIOA, ID_PIOA, PIO_PERIPH_B, PIO_DEFAULT}
/** SPI1 SPCK pin definition. */
#define PIN_SPI1_SPCK     {PIO_PA23B_SPI1_SPCK, PIOA, ID_PIOA, PIO_PERIPH_B, PIO_DEFAULT}
/** SPI1 chip select pin definition. */
#define PIN_SPI1_NPCS0    {PIO_PA8B_SPI1_NPCS0, PIOA, ID_PIOA, PIO_PERIPH_B, PIO_DEFAULT}
/** List of SPI1 pin definitions (MISO, MOSI & SPCK). */
#define PINS_SPI1         PIN_SPI1_MISO, PIN_SPI1_MOSI, PIN_SPI1_SPCK

/** List of all SSC pin definitions. */

/** SSC pin Transmitter Data (TD) */
#define PIN_SSC_TD        {PIO_PA26B_TD, PIOA, ID_PIOA, PIO_PERIPH_B, PIO_DEFAULT}
/** SSC pin Transmitter Clock (TK) */
#define PIN_SSC_TK        {PIO_PA24B_TK, PIOA, ID_PIOA, PIO_PERIPH_B, PIO_DEFAULT}
/** SSC pin Transmitter FrameSync (TF) */
#define PIN_SSC_TF        {PIO_PA25B_TF, PIOA, ID_PIOA, PIO_PERIPH_B, PIO_DEFAULT}
/** SSC pin RD */
#define PIN_SSC_RD        {PIO_PA27B_RD, PIOA, ID_PIOA, PIO_PERIPH_B, PIO_DEFAULT}
/** SSC pin RK */
#define PIN_SSC_RK        {PIO_PA28B_RK, PIOA, ID_PIOA, PIO_PERIPH_B, PIO_DEFAULT}
/** SSC pin RF */
#define PIN_SSC_RF        {PIO_PA29B_RF, PIOA, ID_PIOA, PIO_PERIPH_B, PIO_DEFAULT}
/** SSC pins definition for codec. */
#define PINS_SSC_CODEC    PIN_SSC_TD, PIN_SSC_TK, PIN_SSC_TF, \
                          PIN_SSC_RD, PIN_SSC_RK, PIN_SSC_RF

/** LCD pin list. */
#define PINS_LCD        {0x7DFFFFFF, PIOC, ID_PIOC, PIO_PERIPH_A, PIO_DEFAULT}

/** ADC ADTRG pin (PB18). */
#define PIN_ADTRG       {PIO_PB18B_ADTRG, PIOB, ID_PIOB, PIO_PERIPH_B, PIO_PULLUP}

#endif //_BOARD_
