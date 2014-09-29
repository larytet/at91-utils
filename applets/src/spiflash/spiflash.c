

/*----------------------------------------------------------------------------
 *        Headers
 *----------------------------------------------------------------------------*/
 
#include <board.h>
#include <libspiflash.h>
#include <string.h>
/*----------------------------------------------------------------------------
 *        Internal definitions
 *----------------------------------------------------------------------------*/

/** SPI clock frequency in Hz. */
#define SPCK    12000000


/** Stack size in SRAM */
#define STACK_SIZE 0x100

/** Last erased sector id, this will avoid to erase if the block is already erased. */
static unsigned short lastErasedBlock = 0xFFFF;

/** Indicate the farthest memory offset ever erase, if current write address is less
 than the address, the related block should be erase again before write. */
static uint32_t writtenAddress = 0;


/** Max size of data we can tranfsert in one shot */
#define MAX_COUNT 0xFFFF

/** Chip select value used to select the AT25 chip. */
#define SPI_CS          0
/** SPI peripheral pins to configure to access the serial flash. */
#define SPI_PINS        PINS_SPI0, PIN_SPI0_NPCS0


/*----------------------------------------------------------------------------
 *        Local variables
 *----------------------------------------------------------------------------*/
/** Communication type with SAM-BA GUI.*/
static uint32_t comType;

/** Global DMA driver instance for all DMA transfers in application. */
static sDmad dmad;

/** SPI driver instance. */
static Spid spid;

/** Serial flash driver instance. */
static At25 at25;

/** Pins to configure for the application. */
static Pin pins[] = {SPI_PINS};

/** Size of one page in the serial flash, in bytes.  */
static uint32_t pageSize;

/** Size of one block in the serial flash, in bytes.  */
static uint32_t blockSize;

/**Size of the buffer used for read/write operations in bytes. */
static uint32_t bufferSize;

/**Size of the block to be erased in bytes. */
static uint32_t eraseBlockSize;

void spiflah_init(void)
{
    uint32_t jedecId;
    DMAD_Initialize( &dmad, 1 );
    /* Initialize the SPI and serial flash */
    SPID_Configure(&spid, SPI0, ID_SPI0, &dmad);
    AT25_Configure(&at25, &spid, SPI_CS, 1);

    do
    {
        /* Read the JEDEC ID of the device to identify it */
        jedecId = AT25D_ReadJedecId(&at25);
        if (AT25_FindDevice(&at25, jedecId) == 0)
        {
        	TRACE_DEBUG("Device Unknown %x\n\r", jedecId);
        	break;
        }
        if (AT25D_Unprotect(&at25))
        {
        	TRACE_DEBUG("Can not unprotect the flash\n\r");
            break;
        }

        pageSize = AT25_PageSize(&at25);
        blockSize = AT25_BlockSize(&at25);
        TRACE_DEBUG("Page %d, block %d\n\r", pageSize, blockSize);
    }
    while (0);
}

