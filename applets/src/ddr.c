

#include "configure.h"
#include "board.h"
#include "ddr.h"


#if (BOARD == BOARD_SAMA5D3X)
void ddr_configure_sama5d3x(ddr_type_t device);
#endif

#define DDR2_BA0(r) (1 << (25 + r))
#define DDR2_BA1(r) (1 << (26 + r))


/* -------- MPDDRC_DLL_SOR : (MPDDRC Offset: 0x74) MPDDRC DLL Slave Offset Register -------- */
// SxOFF: DLL Slave x Delay Line Offset ([x=0..1][x=0..3])
#define MPDDRC_DLL_SOR_S0_OFF_Pos 0
#define MPDDRC_DLL_SOR_S0_OFF_Msk (0x1fu << MPDDRC_DLL_SOR_S0_OFF_Pos) /**< \brief (MPDDRC_DLL_SOR) DLL Slave 0 Delay Line Offset */
#define MPDDRC_DLL_SOR_S0_OFF(value) ((MPDDRC_DLL_SOR_S0_OFF_Msk & ((value) << MPDDRC_DLL_SOR_S0_OFF_Pos)))
#define MPDDRC_DLL_SOR_S1_OFF_Pos 8
#define MPDDRC_DLL_SOR_S1_OFF_Msk (0x1fu << MPDDRC_DLL_SOR_S1_OFF_Pos) /**< \brief (MPDDRC_DLL_SOR) DLL Slave 1 Delay Line Offset */
#define MPDDRC_DLL_SOR_S1_OFF(value) ((MPDDRC_DLL_SOR_S1_OFF_Msk & ((value) << MPDDRC_DLL_SOR_S1_OFF_Pos)))
#define MPDDRC_DLL_SOR_S2_OFF_Pos 16
#define MPDDRC_DLL_SOR_S2_OFF_Msk (0x1fu << MPDDRC_DLL_SOR_S2_OFF_Pos) /**< \brief (MPDDRC_DLL_SOR) DLL Slave 2 Delay Line Offset */
#define MPDDRC_DLL_SOR_S2_OFF(value) ((MPDDRC_DLL_SOR_S2_OFF_Msk & ((value) << MPDDRC_DLL_SOR_S2_OFF_Pos)))
#define MPDDRC_DLL_SOR_S3_OFF_Pos 24
#define MPDDRC_DLL_SOR_S3_OFF_Msk (0x1fu << MPDDRC_DLL_SOR_S3_OFF_Pos) /**< \brief (MPDDRC_DLL_SOR) DLL Slave 3 Delay Line Offset */
#define MPDDRC_DLL_SOR_S3_OFF(value) ((MPDDRC_DLL_SOR_S3_OFF_Msk & ((value) << MPDDRC_DLL_SOR_S3_OFF_Pos)))

/**
 * \brief Configures DDR2 (MT47H128M16RT 128MB/ MT47H64M16HR)
 MT47H64M16HR : 8 Meg x 16 x 8 banks
 Refresh count: 8K
 Row address: A[12:0] (8K)
 Column address A[9:0] (1K)
 Bank address BA[2:0] a(24,25) (8)
 */

void ddr_configure_sama5d3x(ddr_type_t device)
{
    volatile uint8_t *pDdr = (uint8_t *) DDR_CS_ADDR;
    volatile uint32_t i;
    volatile uint32_t cr = 0;
    volatile uint32_t dummy_value;
#if 1
    dummy_value = 0x00000000;

    /* Enable DDR2 clock x2 in PMC */
    PMC->PMC_PCER1 = (1 << (ID_MPDDRC-32));
    PMC->PMC_SCER  |= PMC_SCER_DDRCK;
    MPDDRC->MPDDRC_LPR = 0;
    *(uint32_t *)0xFFFFEA24 |= (1 << 5);  // DDRSDRC High Speed Register (MPDDRC_HS)  : hidden option -> calibration during autorefresh
    *(uint32_t *)0xF0038004 |= (0x3 << 16);   // SFR_DDRCFG  DDR Configuration  Force DDR_DQ and DDR_DQS input buffer always on

    MPDDRC->MPDDRC_DLL_SOR = MPDDRC_DLL_SOR_S0_OFF(0x1) | MPDDRC_DLL_SOR_S1_OFF(0x0) | MPDDRC_DLL_SOR_S2_OFF(0x1) | MPDDRC_DLL_SOR_S3_OFF(0x1);
    MPDDRC->MPDDRC_DLL_MOR = (0xC5000000) | MPDDRC_DLL_MOR_MOFF(7) | MPDDRC_DLL_MOR_CLK90OFF(0x1F)  | MPDDRC_DLL_MOR_SELOFF;  // Key = 0xc5000000
    dummy_value  =  MPDDRC->MPDDRC_IO_CALIBR;
    dummy_value &= ~MPDDRC_IO_CALIBR_RDIV_Msk;
    dummy_value &= ~MPDDRC_IO_CALIBR_TZQIO_Msk;
    dummy_value |= MPDDRC_IO_CALIBR_RDIV_RZQ_48;
    dummy_value |= MPDDRC_IO_CALIBR_TZQIO(3);
    MPDDRC->MPDDRC_IO_CALIBR = dummy_value;

   *(uint32_t *)0xF0038004 = (0x3 << 16);   // SFR_DDRCFG  DDR Configuration  Force DDR_DQ and DDR_DQS input buffer always on
#endif
/* Step 1: Program the memory device type */
    /* DBW = 0 (32 bits bus wide); Memory Device = 6 = DDR2-SDRAM = 0x00000006*/
    MPDDRC->MPDDRC_MD = MPDDRC_MD_MD_DDR2_SDRAM;

/* Step 2: Program the features of DDR2-SDRAM device into the Timing Register.*/
    if (device == DDR_TYPE_MT47H128M16RT)
    {
        MPDDRC->MPDDRC_CR = MPDDRC_CR_NR(3)  |
                            MPDDRC_CR_NC(1)  |
                            MPDDRC_CR_CAS(4) |
                            MPDDRC_CR_NB_8 |
                            MPDDRC_CR_DLL_RESET_DISABLED |
                            MPDDRC_CR_DQMS_NOT_SHARED |
                            MPDDRC_CR_ENRDM_OFF |
                            MPDDRC_CR_UNAL_SUPPORTED |
                            MPDDRC_CR_NDQS_DISABLED |
                            MPDDRC_CR_OCD(0x0);
    }
    if (device == DDR_TYPE_MT47H64M16HR)
    {
        MPDDRC->MPDDRC_CR = MPDDRC_CR_NR(2) |
                            MPDDRC_CR_NC(1) |
                            MPDDRC_CR_CAS(3)|
                            MPDDRC_CR_NB_8 |
                            MPDDRC_CR_DLL_RESET_DISABLED |
                            MPDDRC_CR_DQMS_NOT_SHARED |
                            MPDDRC_CR_ENRDM_OFF |
                            MPDDRC_CR_UNAL_SUPPORTED |
                            MPDDRC_CR_NDQS_DISABLED |
                            MPDDRC_CR_OCD(0x0);
    }

    MPDDRC->MPDDRC_TPR0 = MPDDRC_TPR0_TRAS(6)    //  6 * 7.5 = 45 ns
                        | MPDDRC_TPR0_TRCD(2)    //  2 * 7.5 = 15 ns
                        | MPDDRC_TPR0_TWR(2)     //  3 * 7.5 = 22.5 ns
                        | MPDDRC_TPR0_TRC(8)     //  8 * 7.5 = 60 ns
                        | MPDDRC_TPR0_TRP(2)     //  2 * 7.5 = 15 ns
                        | MPDDRC_TPR0_TRRD(1)    //  2 * 7.5 = 15 ns
                        | MPDDRC_TPR0_TWTR(2)    //  2 clock cycle
                        | MPDDRC_TPR0_TMRD(2);   //  2 clock cycles

    MPDDRC->MPDDRC_TPR1 = MPDDRC_TPR1_TRFC(14)   // 18 * 7.5 = 135 ns (min 127.5 ns for 1Gb DDR)
                        | MPDDRC_TPR1_TXSNR(16)  // 20 * 7.5 > 142.5ns TXSNR: Exit self refresh delay to non read command
                        | MPDDRC_TPR1_TXSRD(208) // min 200 clock cycles, TXSRD: Exit self refresh delay to Read command
                        | MPDDRC_TPR1_TXP(2);    //  2 * 7.5 = 15 ns

    MPDDRC->MPDDRC_TPR2 = MPDDRC_TPR2_TXARD(7)   //  min 2 clock cycles
                          | MPDDRC_TPR2_TXARDS(7)//  min 7 clock cycles
                          | MPDDRC_TPR2_TRPA(2)  //  min 18ns
                          | MPDDRC_TPR2_TRTP(2)  //  2 * 7.5 = 15 ns (min 7.5ns)
                          | MPDDRC_TPR2_TFAW(10) ;

    /* DDRSDRC Low-power Register */
    for (i = 0; i < 13300; i++) {
        asm("nop");
    }
    MPDDRC->MPDDRC_LPR = MPDDRC_LPR_LPCB_DISABLED | MPDDRC_LPR_CLK_FR_DISABLED | MPDDRC_LPR_TIMEOUT_0 | MPDDRC_LPR_APDE_FAST;

/* Step 3: An NOP command is issued to the DDR2-SDRAM. Program the NOP command into
    the Mode Register, the application must set MODE to 1 in the Mode Register. */
    MPDDRC->MPDDRC_MR = MPDDRC_MR_MODE_NOP_CMD;
    /* Perform a write access to any DDR2-SDRAM address to acknowledge this command */
    *pDdr = 0;  /* Now clocks which drive DDR2-SDRAM device are enabled.*/

    /* A minimum pause of 200 ��s is provided to precede any signal toggle. (6 core cycles per iteration, core is at 396MHz: min 13200 loops) */
    for (i = 0; i < 13300; i++) {
        asm("nop");
    }

/* Step 4:  An NOP command is issued to the DDR2-SDRAM */
    MPDDRC->MPDDRC_MR = MPDDRC_MR_MODE_NOP_CMD;
    /* Perform a write access to any DDR2-SDRAM address to acknowledge this command.*/
    *pDdr = 0; /* Now CKE is driven high.*/
    /* wait 400 ns min */
    for (i = 0; i < 100; i++) {
        asm("nop");
    }

/* Step 5: An all banks precharge command is issued to the DDR2-SDRAM. */
    MPDDRC->MPDDRC_MR = MPDDRC_MR_MODE_PRCGALL_CMD;
    /* Perform a write access to any DDR2-SDRAM address to acknowledge this command.*/
    *pDdr = 0;
    /* wait 400 ns min */
    for (i = 0; i < 100; i++) {
        asm("nop");
    }

/* Step 6: An Extended Mode Register set (EMRS2) cycle is  issued to chose between commercialor high  temperature operations. */
    MPDDRC->MPDDRC_MR = MPDDRC_MR_MODE_EXT_LMR_CMD;
    *((uint8_t *)(pDdr + DDR2_BA1(device))) = 0; /* The write address must be chosen so that BA[1] is set to 1 and BA[0] is set to 0. */
    /* wait 2 cycles min */
    for (i = 0; i < 100; i++) {
        asm("nop");
    }

/* Step 7: An Extended Mode Register set (EMRS3) cycle is issued to set all registers to 0. */
    MPDDRC->MPDDRC_MR = MPDDRC_MR_MODE_EXT_LMR_CMD;
    *((uint8_t *)(pDdr + DDR2_BA1(device) + DDR2_BA0(device))) = 0;  /* The write address must be chosen so that BA[1] is set to 1 and BA[0] is set to 1.*/
    /* wait 2 cycles min */
    for (i = 0; i < 100; i++) {
        asm("nop");
    }

 /* Step 8:  An Extended Mode Register set (EMRS1) cycle is issued to enable DLL. */
    MPDDRC->MPDDRC_MR = MPDDRC_MR_MODE_EXT_LMR_CMD;
    *((uint8_t *)(pDdr + DDR2_BA0(device))) = 0;  /* The write address must be chosen so that BA[1] is set to 0 and BA[0] is set to 1. */
    /* An additional 200 cycles of clock are required for locking DLL */
    for (i = 0; i < 10000; i++) {
        asm("nop");
    }

/* Step 9:  Program DLL field into the Configuration Register.*/
    cr = MPDDRC->MPDDRC_CR;
    MPDDRC->MPDDRC_CR = cr | MPDDRC_CR_DLL_RESET_ENABLED;

/* Step 10: A Mode Register set (MRS) cycle is issued to reset DLL. */
    MPDDRC->MPDDRC_MR = MPDDRC_MR_MODE_LMR_CMD;
    *(pDdr) = 0;  /* The write address must be chosen so that BA[1:0] bits are set to 0. */
    /* wait 2 cycles min */
    for (i = 0; i < 100; i++) {
        asm("nop");
    }

/* Step 11: An all banks precharge command is issued to the DDR2-SDRAM. */
    MPDDRC->MPDDRC_MR = MPDDRC_MR_MODE_PRCGALL_CMD;
    *(pDdr) = 0;  /* Perform a write access to any DDR2-SDRAM address to acknowledge this command */
    /* wait 2 cycles min */
    for (i = 0; i < 100; i++) {
        asm("nop");
    }

/* Step 12: Two auto-refresh (CBR) cycles are provided. Program the auto refresh command (CBR) into the Mode Register. */
    MPDDRC->MPDDRC_MR = MPDDRC_MR_MODE_RFSH_CMD;
    *(pDdr) = 0;  /* Perform a write access to any DDR2-SDRAM address to acknowledge this command */
    /* wait 2 cycles min */
    for (i = 0; i < 100; i++) {
        asm("nop");
    }
    /* Configure 2nd CBR. */
    MPDDRC->MPDDRC_MR = MPDDRC_MR_MODE_RFSH_CMD;
    *(pDdr) = 0;  /* Perform a write access to any DDR2-SDRAM address to acknowledge this command */
    /* wait 2 cycles min */
    for (i = 0; i < 100; i++) {
        asm("nop");
    }

/* Step 13: Program DLL field into the Configuration Register to low(Disable DLL reset). */
    cr = MPDDRC->MPDDRC_CR;
    MPDDRC->MPDDRC_CR = cr & (~MPDDRC_CR_DLL_RESET_ENABLED);

/* Step 14: A Mode Register set (MRS) cycle is issued to program the parameters of the DDR2-SDRAM devices. */
    MPDDRC->MPDDRC_MR = MPDDRC_MR_MODE_LMR_CMD;
    *(pDdr) = 0;  /* The write address must be chosen so that BA[1:0] are set to 0. */
    /* wait 2 cycles min */
    for (i = 0; i < 100; i++) {
        asm("nop");
    }

/* Step 15: Program OCD field into the Configuration Register to high (OCD calibration default). */
    cr = MPDDRC->MPDDRC_CR;
    MPDDRC->MPDDRC_CR = cr | MPDDRC_CR_OCD(0x07);

/* Step 16: An Extended Mode Register set (EMRS1) cycle is issued to OCD default value. */
    MPDDRC->MPDDRC_MR = MPDDRC_MR_MODE_EXT_LMR_CMD;
    *((uint8_t *)(pDdr + DDR2_BA0(device))) = 0;  /* The write address must be chosen so that BA[1] is set to 0 and BA[0] is set to 1.*/
    /* wait 2 cycles min */
    for (i = 0; i < 100; i++) {
        asm("nop");
    }

/* Step 17: Program OCD field into the Configuration Register to low (OCD calibration mode exit). */
   // cr = MPDDRC->MPDDRC_CR;
   // MPDDRC->MPDDRC_CR = cr  & (~ MPDDRC_CR_OCD(0x07));

/* Step 18: An Extended Mode Register set (EMRS1) cycle is issued to enable OCD exit.*/
    MPDDRC->MPDDRC_MR = MPDDRC_MR_MODE_EXT_LMR_CMD;
    *((uint8_t *)(pDdr + DDR2_BA0(device))) = 0;  /* The write address must be chosen so that BA[1] is set to 0 and BA[0] is set to 1.*/
    /* wait 2 cycles min */
    for (i = 0; i < 100; i++) {
        asm("nop");
    }

/* Step 19,20: A mode Normal command is provided. Program the Normal mode into Mode Register. */
    MPDDRC->MPDDRC_MR = MPDDRC_MR_MODE_NORMAL_CMD;
    *(pDdr) = 0;

/* Step 21: Write the refresh rate into the count field in the Refresh Timer register. The DDR2-SDRAM device requires a refresh every 15.625 ��s or 7.81 ��s.
   With a 100MHz frequency, the refresh timer count register must to be set with (15.625 /100 MHz) = 1562 i.e. 0x061A or (7.81 /100MHz) = 781 i.e. 0x030d. */
    /* For MT47H64M16HR, The refresh period is 64ms (commercial), This equates to an average
       refresh rate of 7.8125��s (commercial), To ensure all rows of all banks are properly
       refreshed, 8192 REFRESH commands must be issued every 64ms (commercial) */
    /* ((64 x 10(^-3))/8192) x133 x (10^6) */
    MPDDRC->MPDDRC_RTR = MPDDRC_RTR_COUNT(300); /* Set Refresh timer 7.8125 us*/
    /* OK now we are ready to work on the DDRSDR */
    /* wait for end of calibration */
    for (i = 0; i < 500; i++) {
        asm("    nop");
    }
}


/**
 * Configure DDR2
 */
void ddr_configure(ddr_type_t ddr_type)
{
#if (BOARD == BOARD_SAMA5D3X)
	ddr_configure_sama5d3x(ddr_type);
#endif
}

/**
 * Run a couple of quick DDR tests
 */
void ddr_test()
{
#if CONFIGURE_DDR_TEST
    uint32_t *ptr;
    int i;
    uint32_t size = CONFIGURE_DDR_TEST_SIZE;

    ptr = (uint32_t *) DDR_CS_ADDR;

    for (i = 0; i < size; ++i) {

        if (i & 1) {
            ptr[i] = 0x55AA55AA | (1 << i);
        }
        else {
            ptr[i] = 0xAA55AA55 | (1 << i);
        }
    }

    for (i = 0; i < size; ++i) {
        if (i & 1) {
            if (ptr[i] != (0x55AA55AA | (1 << i))) {
                return ;
            }
        }
        else {
            if (ptr[i] != (0xAA55AA55 | (1 << i))) {
                return ;
            }
        }
    }


    for (i = 0; i < size; ++i)
    {
        ptr[i] = (uint32_t)(&ptr[i]);
    }

    for (i = 0; i < size; ++i) {
        if (ptr[i] != (uint32_t)(&ptr[i]))
		{
        	return;
		}
    }

    for (i = 0; i < 10; ++i)
    {
        ptr[i] = 0x0E80F10E;  // Test Fine
    }

#endif
}
