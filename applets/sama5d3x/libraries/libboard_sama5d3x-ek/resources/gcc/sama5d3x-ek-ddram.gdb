#------------------------------------------------
# DDRAM initialization script for the AT91SAM9xx5
#------------------------------------------------

# Step1: Connect to the J-Link gdb server
define reset
target remote localhost:2331
monitor reset

# Step2: Reset peripheral  (RSTC_CR)
#set *0xFFFFFE00 = 0xA5000004

echo Switch to PLL + prescaler (1)
set $read  = *0xFFFFFC30
set $read  &= ~(0x3)
set $read  |= 1
set *0xFFFFFC30 = $read
while ((*0xFFFFFC68 & 0x8) == 0)
end

echo Switch to main Osc (2)
set *0xFFFFFC20 = 0x01374009
while ((*0xFFFFFC68 & 0x1) == 0)
end

echo  PLLA 0
set *0xFFFFFC28 = 0
   
echo Initialize PLLA
set *0xFFFFFC28 = 0x20C73F03
while ((*0xFFFFFC68 & 0x2) == 0)
end
while ((*0xFFFFFC68 & 0x8) == 0)
end

echo Switch to main oscillator + prescaler (3)
set $read  = *0xFFFFFC30
set $read  &= ~(0x3 << 8)
set $read  |= ((0x3 << 8) | (0x1 << 12))
set *0xFFFFFC30 = $read
while ((*0xFFFFFC68 & 0x8) == 0)
end

echo Switch to main oscillator + prescaler (4)
set $read  = *0xFFFFFC30
set $read  &= ~(0x7 << 4)
set $read  |= (0x0 << 4)
set *0xFFFFFC30 = $read
while ((*0xFFFFFC68 & 0x8) == 0)
end

echo Switch to PLL + prescaler (5)
set $read  = *0xFFFFFC30
set $read  &= ~(0x3)
set $read  |= 2
set *0xFFFFFC30 = $read
while ((*0xFFFFFC68 & 0x8) == 0)
end

echo Master clock ok.\n

  

echo Configuring the DDRAM controller...\n

# *ADDR_CCFG_EBICSA = 0x01020002; /* EBI Chip Select 1 is assigned to the DDR2SDR Controller 
set *0xFFFFDF20 = 0x01020002
 
 # Enable DDR2 clock x2 in PMC 
set *0xFFFFFC00 = 0x04

# Disable anticipated read
set *0xFFFFE82C |= 0x04

# -----------------------Step 1------------------- 
# Program the memory device type
# ------------------------------------------------ 

#DDRSDRC->DDRSDRC_MD = DDRSDRC_MD_MD(DDR2_SDRAM) | DDRSDRC_MD_DBW;
set *0xFFFFE820 = 0x16
    

# -----------------------Step 2------------------- 
# 1. Program the features of DDR2-SDRAM device into 
#    the Configuration Register.
# 2. Program the features of DDR2-SDRAM device into 
#    the Timing Register HDDRSDRC2_T0PR.    
# 3. Program the features of DDR2-SDRAM device into 
#    the Timing Register HDDRSDRC2_T1PR.
# 4. Program the features of DDR2-SDRAM device into 
#    the Timing Register HDDRSDRC2_T2PR.
# ------------------------------------------------ 

# DDRSDRC->DDRSDRC_CR = DDRSDRC_CR_NC(NC_DDR10_SDR9)        // 10 column bits (1K)
#                            | DDRSDRC_CR_NR(NR_ROW_BIT_13) // 13 row bits    (8K)
#                            | DDRSDRC_CR_CAS(3)            // CAS Latency 3
#                            ;//| (0x1 << 20);              // (DDRSDRC) DDR2 8 bank
    
set *0xFFFFE808 = 0x0000B9

#  DDRSDRC->DDRSDRC_T0PR = DDRSDRC_T0PR_TRAS(6)    //  6 * 7.5 = 45 ns
#                          | DDRSDRC_T0PR_TRCD(2)    //  2 * 7.5 = 15 ns
#                          | DDRSDRC_T0PR_TWR(2)     //  2 * 7.5 = 15 ns
#                          | DDRSDRC_T0PR_TRC(8)     //  8 * 7.5 = 60 ns
#                          | DDRSDRC_T0PR_TRP(2)     //  2 * 7.5 = 15 ns
#                          | DDRSDRC_T0PR_TRRD(1)    //  2 * 7.5 = 15 ns
#                          | DDRSDRC_T0PR_TWTR(1)    //  2 clock cycle
#                          | DDRSDRC_T0PR_TMRD(2);   //  2 clock cycles
set *0xFFFFE80C = 0x21128226

# DDRSDRC->DDRSDRC_T1PR = DDRSDRC_T1PR_TRFC(18)   // 18 * 7.5 = 135 ns (min 127.5 ns for 1Gb DDR)
#                          | DDRSDRC_T1PR_TXSNR(19)  // 19 * 7.5 > 142.5ns TXSNR: Exit self refresh delay to non read command
#                          | DDRSDRC_T1PR_TXSRD(200) // min 200 clock cycles, TXSRD: Exit self refresh delay to Read command
#                          | DDRSDRC_T1PR_TXP(2);    //  2 * 7.5 = 15 ns
set *0xFFFFE810 = 0x02c81312

#    DDRSDRC->DDRSDRC_T2PR = DDRSDRC_T2PR_TXARD(2)   //  min 2 clock cycles
#                          | DDRSDRC_T2PR_TXARDS(7)  //  min 7 clock cycles
#                          | DDRSDRC_T2PR_TRPA(3)
#                          | DDRSDRC_T2PR_TRTP(1) ;  //  1 * 7.5 = 7.5 ns (min 7.5ns)
set *0xFFFFE814 = 0x0001372

# -----------------------Step 3------------------- 
# An NOP command is issued to the DDR2-SDRAM to 
# enable clock.
# ------------------------------------------------ 
set *0xFFFFE800 = 0x1
set *0x20000000 = 0

# A minimum pause of 200 ¦Ìs is provided to precede any signal toggle.

set $i = 0
while $i != 13300
  set $i += 1
end

# Now clocks which drive DDR2-SDRAM device are enabled

# -----------------------Step 4------------------- 
# An NOP command is issued to the DDR2-SDRAM 
# ------------------------------------------------ 
set *0xFFFFE800 = 0x1
set *0x20000000 = 0
# Now CKE is driven high.
set $i = 0
while $i != 100
  set $i += 1
end

# -----------------------Step 5------------------- 
# An all banks precharge command is issued to the 
# DDR2-SDRAM.
# ------------------------------------------------ 
# HDDRSDRC2_MR, AT91C_DDRC2_MODE_PRCGALL_CMD  
set *0xFFFFE800 = 0x2
set *0x20000000 = 0
   
set $i = 0
while $i != 100
  set $i += 1
end

# -----------------------Step 6------------------- 
# An Extended Mode Register set (EMRS2) cycle is 
# issued to chose between commercialor high 
# temperature operations
# ------------------------------------------------ 

# HDDRSDRC2_MR, AT91C_DDRC2_MODE_EXT_LMR_CMD  
set *0xFFFFE800 = 0x5
set *0x22000000 = 0
set $i = 0
while $i != 100
  set $i += 1
end


# -----------------------Step 7------------------- 
# An Extended Mode Register set (EMRS3) cycle is 
# issued to set all registers to 0.
# ------------------------------------------------
# HDDRSDRC2_MR, AT91C_DDRC2_MODE_EXT_LMR_CMD  
set *0xFFFFE800 = 0x5
set *0x23000000 = 0
set $i = 0
while $i != 100
  set $i += 1
end

# -----------------------Step 8------------------- 
# An Extended Mode Register set (EMRS1) cycle is 
# issued to enable DLL.
# ------------------------------------------------
# HDDRSDRC2_MR, AT91C_DDRC2_MODE_EXT_LMR_CMD  
set *0xFFFFE800 = 0x5
set *0x21000000 = 0
 # An additional 200 cycles of clock are required for locking DLL
set $i = 0
while $i != 10000
  set $i += 1
end


# -----------------------Step 9------------------- 
# Program DLL field into the Configuration Register.
# -------------------------------------------------

# HDDRSDRC2_CR, cr | AT91C_DDRC2_DLL_RESET_ENABLED
set *0xFFFFE808 |= 0x80

# -----------------------Step 10------------------- 
# A Mode Register set (MRS) cycle is issued to reset
# DLL.
# -------------------------------------------------
# HDDRSDRC2_MR, AT91C_DDRC2_MODE_LMR_CMD
set *0xFFFFE800 = 0x3
set *0x20000000 = 0
set $i = 0
while $i != 100
  set $i += 1
end

# -----------------------Step 11------------------- 
# An all banks precharge command is issued to the 
# DDR2-SDRAM.
# -------------------------------------------------
# HDDRSDRC2_MR, AT91C_DDRC2_MODE_PRCGALL_CMD
set *0xFFFFE800 = 0x2
set *0x20000000 = 0
set $i = 0
while $i != 100
  set $i += 1
end

# -----------------------Step 12------------------- 
# Two auto-refresh (CBR) cycles are provided. 
# Program the auto refresh command (CBR) into the 
# Mode Register.
# -------------------------------------------------
# HDDRSDRC2_MR, AT91C_DDRC2_MODE_RFSH_CMD
set *0xFFFFE800 = 0x4
set *0x20000000 = 0
set $i = 0
while $i != 100
  set $i += 1
end

# Set 2nd CBR
# HDDRSDRC2_MR, AT91C_DDRC2_MODE_RFSH_CMD
set *0xFFFFE800 = 0x4
set *0x20000000 = 0
set $i = 0
while $i != 100
  set $i += 1
end

# -----------------------Step 13------------------- 
# Program DLL field into the Configuration Register
# to low(Disable DLL reset).
# -------------------------------------------------
# HDDRSDRC2_CR, cr & (~AT91C_DDRC2_DLL_RESET_ENABLED)  
set *0xFFFFE808 &= 0xFFFFFF7F


# -----------------------Step 14------------------- 
# A Mode Register set (MRS) cycle is issued to 
# program the parameters of the DDR2-SDRAM devices
# -------------------------------------------------
# HDDRSDRC2_MR, AT91C_DDRC2_MODE_LMR_CMD
set *0xFFFFE800 = 0x3
set *0x20000000 = 0
set $i = 0
while $i != 100
  set $i += 1
end

# -----------------------Step 15------------------- 
# Program OCD field into the Configuration Register
# to high (OCD calibration default)
# -------------------------------------------------
set *0xFFFFE808  |= (0x07 << 12)

# -----------------------Step 16------------------- 
# An Extended Mode Register set (EMRS1) cycle is 
# issued to OCD default value.
# -------------------------------------------------
# HDDRSDRC2_MR, AT91C_DDRC2_MODE_EXT_LMR_CMD
set *0xFFFFE800 = 0x5
set *0x21000000 = 0
set $i = 0
while $i != 100
  set $i += 1
end

# -----------------------Step 17------------------- 
# Program OCD field into the Configuration Register 
# to low (OCD calibration mode exit).
# -------------------------------------------------
set *0xFFFFE808  &= 0xFFFF8FFF
 
# -----------------------Step 18------------------- 
# An Extended Mode Register set (EMRS1) cycle is 
# issued to enable OCD exit.
# -------------------------------------------------
# HDDRSDRC2_MR, AT91C_DDRC2_MODE_EXT_LMR_CMD
set *0xFFFFE800 = 0x5
set *0x21000000 = 0
set $i = 0
while $i != 100
  set $i += 1
end

# -----------------------Step 19,20------------------- 
# A mode Normal command is provided. Program the 
# Normal mode into Mode Register.
# -------------------------------------------------
# HDDRSDRC2_MR, AT91C_DDRC2_MODE_NORMAL_CMD
set *0xFFFFE800 = 0x0
set *0x20000000 = 0
set $i = 0
while $i != 100
  set $i += 1
end

# -----------------------Step 21------------------- 
# Write the refresh rate into the count field in the 
# Refresh Timer register. The DDR2-SDRAM device requires a
# refresh every 15.625 ¦Ìs or 7.81 ¦Ìs. With a 100 
# MHz frequency, the refresh timer count register must to 
# be set with (15.625 /100 MHz) = 1562 i.e. 0x061A or 
# (7.81 /100MHz) = 781 i.e. 0x030d
# -------------------------------------------------
# HDDRSDRC2_RTR, 0x00000411
set *0xFFFFE804 = 0x00000411
# Read optimization" shall be un-selected on this revision.
set *0xFFFFE82C = 0x04
    
# OK now we are ready to work on the DDRSDR

echo DDRAM configuration ok.\n



# Step3: Load file(eg. getting-started project)
load

# toggle remap bits
set *0xFFFFE100 = 0x03

mon reg pc=0x20000000
info reg

end