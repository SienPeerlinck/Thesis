#include "sam.h"
#include "FreeRTOS.h"

#include "ext_flash.h"

#define BOOTFUNC __attribute__((section(".boot")))

#define FLEXRAM_ADDR   0x21000000
#define FLEXRAM_SIZE   0x000C0000
#define ITCM_ADDR      0x00000000
#define ITCM_SIZE      0x00020000
#define DTCM_ADDR      0x20000000
#define DTCM_SIZE      0x00040000
#define EXT_FLASH_ADDR 0x60000000

#define MEM_ZONE_MASK  0xff000000

#define TRANSFER_CHUNK_SIZE 0x80

// Constant defs for boot_status
#define BOOT_FROM_EMBEDDED_FLASH 0
#define BOOT_FROM_EXTERNAL_FLASH 1

char boot_status;
unsigned int boot_pc;
unsigned int reset_status;

/* Initialize segments */
extern unsigned int _estack;
extern unsigned int _vec_table;
extern unsigned int _e_vec_table;
extern unsigned int _vec_table_itcm;
extern unsigned int _etext;
extern unsigned int _srelocate_d;
extern unsigned int _erelocate_d;
extern unsigned int _srelocate_i;
extern unsigned int _erelocate_i;
extern unsigned int _szero;
extern unsigned int _ezero;

BOOTFUNC
void boot(void);

int main(void);

/* Reset handler */
// void Reset_Handler(void);

/* Default empty handler */
void Dummy_Handler(void);

/* Cortex-M7 core handlers */
void NonMaskableInt_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void HardFault_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void MemoryManagement_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void BusFault_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void UsageFault_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void SVCall_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void DebugMonitor_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void PendSV_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void SysTick_Handler(void) __attribute__((weak, alias("Dummy_Handler")));

/* Device vectors list dummy definition*/
void SUPC_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void RSTC_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void RTC_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void RTT_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void WDT_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void PMC_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void MATRIX0_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void FLEXCOM0_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void FLEXCOM1_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void NMIC_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void PIOA_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void PIOB_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void PIOC_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void FLEXCOM2_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void FLEXCOM3_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void FLEXCOM4_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void PIOD_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void PIOE_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void CCW_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void CCF_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void FPU_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void IXC_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void FLEXCOM5_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void FLEXCOM6_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void FLEXCOM7_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void TC0_CH0_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void TC0_CH1_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void TC0_CH2_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void TC1_CH0_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void TC1_CH1_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void TC1_CH2_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void PWM0_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void PWM1_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void ICM_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void PIOF_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void PIOG_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void MCAN0_INT0_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void MCAN0_INT1_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void MCAN1_INT0_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void MCAN1_INT1_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void TCMECC_INTFIX_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void TCMECC_INTNOFIX_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void FLEXRAMECC_INTFIX_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void FLEXRAMECC_INTNOFIX_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void SHA_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void FLEXCOM8_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void FLEXCOM9_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void RSWDT_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void QSPI_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void HEFC_INT0_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void HEFC_INTFIX_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void HEFC_INTNOFIX_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void TC2_CH0_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void TC2_CH1_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void TC2_CH2_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void TC3_CH0_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void TC3_CH1_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void TC3_CH2_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void HEMC_INTSDRAMC_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void HEMC_INTFIX_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void HEMC_INTNOFIX_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void SFR_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void TRNG_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void XDMAC_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void SPW_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void IP1553_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void GMAC_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void GMAC_Q1_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void GMAC_Q2_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void GMAC_Q3_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void GMAC_Q4_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void GMAC_Q5_Handler(void) __attribute__((weak, alias("Dummy_Handler")));

/* Multiple handlers for vector */
//__attribute__((section(".vectors_itcm")))
//DeviceVectors exception_table_itcm;

__attribute__((section(".vectors")))
const DeviceVectors exception_table =
    {
        /* Configure Initial Stack Pointer, using linker-generated symbols */
        .pvStack = &_estack,

        .pfnReset_Handler = Reset_Handler,
        .pfnNonMaskableInt_Handler = NonMaskableInt_Handler,
        .pfnHardFault_Handler = HardFault_Handler,
        .pfnMemoryManagement_Handler = MemoryManagement_Handler,
        .pfnBusFault_Handler = BusFault_Handler,
        .pfnUsageFault_Handler = UsageFault_Handler,
        .pfnSVCall_Handler = SVCall_Handler,
        .pfnDebugMonitor_Handler = DebugMonitor_Handler,
        .pfnPendSV_Handler = PendSV_Handler,
        .pfnSysTick_Handler = SysTick_Handler,
        .pfnSUPC_Handler = SUPC_Handler,
        .pfnRSTC_Handler = RSTC_Handler,
        .pfnRTC_Handler = RTC_Handler,
        .pfnRTT_Handler = RTT_Handler,
        .pfnWDT_Handler = WDT_Handler,
        .pfnPMC_Handler = PMC_Handler,
        .pfnMATRIX0_Handler = MATRIX0_Handler,
        .pfnFLEXCOM0_Handler = FLEXCOM0_Handler,
        .pfnFLEXCOM1_Handler = FLEXCOM1_Handler,
        .pfnNMIC_Handler = NMIC_Handler,
        .pfnPIOA_Handler = PIOA_Handler,
        .pfnPIOB_Handler = PIOB_Handler,
        .pfnPIOC_Handler = PIOC_Handler,
        .pfnFLEXCOM2_Handler = FLEXCOM2_Handler,
        .pfnFLEXCOM3_Handler = FLEXCOM3_Handler,
        .pfnFLEXCOM4_Handler = FLEXCOM4_Handler,
        .pfnPIOD_Handler = PIOD_Handler,
        .pfnPIOE_Handler = PIOE_Handler,
        .pfnCCW_Handler = CCW_Handler,
        .pfnCCF_Handler = CCF_Handler,
        .pfnFPU_Handler = FPU_Handler,
        .pfnIXC_Handler = IXC_Handler,
        .pfnFLEXCOM5_Handler = FLEXCOM5_Handler,
        .pfnFLEXCOM6_Handler = FLEXCOM6_Handler,
        .pfnFLEXCOM7_Handler = FLEXCOM7_Handler,
        .pfnTC0_CH0_Handler = TC0_CH0_Handler,
        .pfnTC0_CH1_Handler = TC0_CH1_Handler,
        .pfnTC0_CH2_Handler = TC0_CH2_Handler,
        .pfnTC1_CH0_Handler = TC1_CH0_Handler,
        .pfnTC1_CH1_Handler = TC1_CH1_Handler,
        .pfnTC1_CH2_Handler = TC1_CH2_Handler,
        .pfnPWM0_Handler = PWM0_Handler,
        .pfnPWM1_Handler = PWM1_Handler,
        .pfnICM_Handler = ICM_Handler,
        .pfnPIOF_Handler = PIOF_Handler,
        .pfnPIOG_Handler = PIOG_Handler,
        .pfnMCAN0_INT0_Handler = MCAN0_INT0_Handler,
        .pfnMCAN0_INT1_Handler = MCAN0_INT1_Handler,
        .pfnMCAN1_INT0_Handler = MCAN1_INT0_Handler,
        .pfnMCAN1_INT1_Handler = MCAN1_INT1_Handler,
        .pfnTCMECC_INTFIX_Handler = TCMECC_INTFIX_Handler,
        .pfnTCMECC_INTNOFIX_Handler = TCMECC_INTNOFIX_Handler,
        .pfnFLEXRAMECC_INTFIX_Handler = FLEXRAMECC_INTFIX_Handler,
        .pfnFLEXRAMECC_INTNOFIX_Handler = FLEXRAMECC_INTNOFIX_Handler,
        .pfnSHA_Handler = SHA_Handler,
        .pfnFLEXCOM8_Handler = FLEXCOM8_Handler,
        .pfnFLEXCOM9_Handler = FLEXCOM9_Handler,
        .pfnRSWDT_Handler = RSWDT_Handler,
        .pfnQSPI_Handler = QSPI_Handler,
        .pfnHEFC_INT0_Handler = HEFC_INT0_Handler,
        .pfnHEFC_INTFIX_Handler = HEFC_INTFIX_Handler,
        .pfnHEFC_INTNOFIX_Handler = HEFC_INTNOFIX_Handler,
        .pfnTC2_CH0_Handler = TC2_CH0_Handler,
        .pfnTC2_CH1_Handler = TC2_CH1_Handler,
        .pfnTC2_CH2_Handler = TC2_CH2_Handler,
        .pfnTC3_CH0_Handler = TC3_CH0_Handler,
        .pfnTC3_CH1_Handler = TC3_CH1_Handler,
        .pfnTC3_CH2_Handler = TC3_CH2_Handler,
        .pfnHEMC_INTSDRAMC_Handler = HEMC_INTSDRAMC_Handler,
        .pfnHEMC_INTFIX_Handler = HEMC_INTFIX_Handler,
        .pfnHEMC_INTNOFIX_Handler = HEMC_INTNOFIX_Handler,
        .pfnSFR_Handler = SFR_Handler,
        .pfnTRNG_Handler = TRNG_Handler,
        .pfnXDMAC_Handler = XDMAC_Handler,
        .pfnSPW_Handler = SPW_Handler,
        .pfnIP1553_Handler = IP1553_Handler,
        .pfnGMAC_Handler = GMAC_Handler,
        .pfnGMAC_Q1_Handler = GMAC_Q1_Handler,
        .pfnGMAC_Q2_Handler = GMAC_Q2_Handler,
        .pfnGMAC_Q3_Handler = GMAC_Q3_Handler,
        .pfnGMAC_Q4_Handler = GMAC_Q4_Handler,
        .pfnGMAC_Q5_Handler = GMAC_Q5_Handler,

};

BOOTFUNC
inline static void Read_Chunk(uint32_t address)
{
  __asm__ volatile(
      "MOV      r8, %[addr]\n"
      "PLD      [r8, #0xC0]\n"
      "VLDM     r8,{d0-d15}\n"
      :
      : [addr] "l"(address)
      : "r8");
}

BOOTFUNC
inline static void Write_Chunk(uint32_t address)
{
  __asm__ volatile(
      "MOV      r8, %[addr]\n"
      "VSTM     r8,{d0-d15}\n"
      :
      : [addr] "l"(address)
      : "r8");
}

BOOTFUNC
inline static void FlexRAM_EccInitialize(void)
{
  uint32_t pFlexRam;

  __DSB();
  __ISB();

  // FlexRAM initialization loop (to handle ECC properly)
  for (pFlexRam = FLEXRAM_ADDR; pFlexRam < (FLEXRAM_ADDR + FlexRAM_SIZE); pFlexRam += TRANSFER_CHUNK_SIZE)
  {
    Read_Chunk(pFlexRam);
    __DSB();
    __ISB();
    Write_Chunk(pFlexRam);
    __DSB();
    __ISB();
  }

  __DSB();
  __ISB();
}

/*
BOOTFUNC
__attribute__ ((__noinline__))
void * get_pc () { return __builtin_return_address(0); }
*/

BOOTFUNC
static __inline__ void * get_pc(void)  {
    void *pc;
    asm("mov %0, pc" : "=r"(pc));
    return pc;
}

BOOTFUNC
static void init_pmc(void)
{
  unsigned int mainf;

  // from Datasheet
  // 27.13 Recommanded Programming Sequence
 
  // Step 2 & 3
  // Main Crystal Oscillator Bypass;
  PMC_REGS->CKGR_MOR = CKGR_MOR_KEY_PASSWD | CKGR_MOR_MOSCSEL(1) | CKGR_MOR_MOSCXTBY(1);

  // Step 4
  // Wait for PMC_SR.MOSCSELS == 1 to ensure the switch is complete
  while ((PMC_REGS->PMC_SR & PMC_SR_MOSCSELS_Msk) == 0)
    ;

  // Step 5
  // Check Main Clock frequency
  PMC_REGS->CKGR_MCFR = CKGR_MCFR_CCSS(1);
  while ((PMC_REGS->CKGR_MCFR & CKGR_MCFR_MAINFRDY_Msk) == 0)
    ;
  //mainf =  PMC_REGS->CKGR_MCFR & CKGR_MCFR_MAINF_Msk;
  // FixMe: if measured mainf is not correct ==> TBD
  //        ??? Continue with Main RC oscillator ???

  // Step 6
  // PLLACK = MAINCK * (MULA+1) / DIVA
  // 100MHz = 10MHz * 10 / 1
  PMC_REGS->CKGR_PLLAR = CKGR_PLLAR_ONE(1) | CKGR_PLLAR_MULA(10 - 1) | CKGR_PLLAR_FREQ_VCO(1) | CKGR_PLLAR_PLLACOUNT(63) | CKGR_PLLAR_DIVA(1);

  // Wait for the PLL to be locked
  while ((PMC_REGS->PMC_SR & PMC_SR_LOCKA_Msk) == 0)
    ;

  // HCLK = PLLACK / 2
  //      = 50 MHz
  // MCLK = HCLK = 50 MHz
  // Step 7a
  PMC_REGS->PMC_MCKR = PMC_MCKR_CSS_MAIN_CLK | PMC_MCKR_PRES_CLK_2 | PMC_MCKR_MDIV_EQ_PCK;
  // Step 7b
  // Wait for MCLK to be ready
  while ((PMC_REGS->PMC_SR & PMC_SR_MCKRDY_Msk) == 0)
    ;

  // Step 7e
  PMC_REGS->PMC_MCKR = PMC_MCKR_CSS_PLLA_CLK | PMC_MCKR_PRES_CLK_2 | PMC_MCKR_MDIV_EQ_PCK;
  // Step 7f
  // Wait for MCLK to be ready
  while ((PMC_REGS->PMC_SR & PMC_SR_MCKRDY_Msk) == 0)
    ;
    
  // Disable the Main RC oscillator
  PMC_REGS->CKGR_MOR &= ~CKGR_MOR_MOSCRCEN_Msk;

}
uint32_t SystemCoreClock = 50000000;

BOOTFUNC
inline static void TCM_EccInitialize(void)
{
  uint32_t pTCM;

  __DSB();
  __ISB();


  SCB->ITCMCR = SCB_ITCMCR_EN_Msk; 
  SCB->DTCMCR = SCB_DTCMCR_EN_Msk; 

  __DSB();
  __ISB();

  for (pTCM = ITCM_ADDR; pTCM < (ITCM_ADDR + ITCM_SIZE); pTCM += TRANSFER_CHUNK_SIZE)
  {
    TCMECC_REGS->TCMECC_CR = TCMECC_CR_ENABLE(0); // Disable TCMHECC
    __DSB();
    __ISB();
    Read_Chunk(pTCM);
    __DSB();
    __ISB();
    TCMECC_REGS->TCMECC_CR = TCMECC_CR_ENABLE(1); // Enable TCMHECC
    __DSB();
    __ISB();
    Write_Chunk(pTCM);
    __DSB();
    __ISB();
  }

  for (pTCM = DTCM_ADDR; pTCM < (DTCM_ADDR + DTCM_SIZE); pTCM += TRANSFER_CHUNK_SIZE)
  {
    TCMECC_REGS->TCMECC_CR = TCMECC_CR_ENABLE(0); // Disable TCMHECC
    __DSB();
    __ISB();
    Read_Chunk(pTCM);
    __DSB();
    __ISB();
    TCMECC_REGS->TCMECC_CR = TCMECC_CR_ENABLE(1); // Enable TCMHECC
    __DSB();
    __ISB();
    Write_Chunk(pTCM);
    __DSB();
    __ISB();
  }

  TCMECC_REGS->TCMECC_CR = TCMECC_CR_RST_FIX_CPT(1) | TCMECC_CR_RST_NOFIX_CPT(1) | TCMECC_CR_ENABLE(1);

  __DSB();
  __ISB();
  SCB->ITCMCR = (8 << SCB_ITCMCR_SZ_Pos) | SCB_ITCMCR_EN_Msk |  SCB_ITCMCR_RMW_Msk;// | SCB_ITCMCR_RETEN_Msk;
  SCB->DTCMCR = (9 << SCB_DTCMCR_SZ_Pos) | SCB_DTCMCR_EN_Msk |  SCB_DTCMCR_RMW_Msk;// | SCB_DTCMCR_RETEN_Msk;
  __DSB();
  __ISB();
}


BOOTFUNC
void boot(void)
{
  unsigned int *p_src, *p_dest;
  unsigned int *p_src_end;
  unsigned int tmp_boot_pc;
  char tmp_boot_status;

  // Disable Watchdog
  //WDT_REGS->WDT_MR = WDT_MR_WDDIS(1);

  // Clock initialization
  init_pmc();

  // Processor is now running at 50 MHz

  __DSB();
  __ISB();
  // Memory timings for External Flash, with MCK = 50MHz ==> unit = 20ns
  HSMC_REGS->HSMC_CS[0].HSMC_SETUP = HSMC_SETUP_NWE_SETUP(2) | HSMC_SETUP_NCS_WR_SETUP(1) |
                                       HSMC_SETUP_NRD_SETUP(2) | HSMC_SETUP_NCS_RD_SETUP(1);
  HSMC_REGS->HSMC_CS[0].HSMC_PULSE = HSMC_PULSE_NWE_PULSE(4) | HSMC_PULSE_NCS_WR_PULSE(6) |
                                       HSMC_PULSE_NRD_PULSE(4) | HSMC_PULSE_NCS_RD_PULSE(6);
  HSMC_REGS->HSMC_CS[0].HSMC_CYCLE = HSMC_CYCLE_NWE_CYCLE(8) | HSMC_CYCLE_NRD_CYCLE(8);

  // Enable FPU
  SCB->CPACR |= 0xf << 20;
  __DSB();
  __ISB();

  TCM_EccInitialize();

  FlexRAM_EccInitialize();

  // Set boot_status
  tmp_boot_pc = (unsigned int)get_pc();
  if ( ((unsigned int)get_pc() & MEM_ZONE_MASK) == EXT_FLASH_ADDR)
    tmp_boot_status = BOOT_FROM_EXTERNAL_FLASH;
  else
    tmp_boot_status = BOOT_FROM_EMBEDDED_FLASH;


  // Copy Vector table
  p_src = &_vec_table;
  p_src_end = &_e_vec_table;
  if (tmp_boot_status) {
    // We are running in external Flash ==> Fix p_src
    p_src = (void*)(((unsigned int)p_src & ~MEM_ZONE_MASK) | EXT_FLASH_ADDR);
    p_src_end = (void*)(((unsigned int)p_src_end & ~MEM_ZONE_MASK) | EXT_FLASH_ADDR);
  }

  p_dest = &_vec_table_itcm;
  while (p_src < p_src_end)
  {
    *p_dest++ = *p_src++;
  }

  /* Initialize the relocate segment */
  p_src = &_etext;
  if (tmp_boot_status) {
    // We are running in external Flash ==> Fix p_src
    p_src = (void*)(((unsigned int)p_src & ~MEM_ZONE_MASK) | EXT_FLASH_ADDR);
  }

  p_dest = &_srelocate_d;
  while (p_dest < &_erelocate_d)
  {
    *p_dest++ = *p_src++;
  }
  

  p_dest = &_srelocate_i;
  while (p_dest < &_erelocate_i)
  {
    *p_dest++ = *p_src++;
  }
  

  /* Clear the zero segment */
  p_dest = &_szero;
  while (p_dest < &_ezero)
  {
    *p_dest++ = 0;
  }

  boot_pc = tmp_boot_pc;
  boot_status = tmp_boot_status;
  reset_status = RSTC_REGS->RSTC_SR;

  /* Set the vector table base address */
  p_src = (unsigned int *)&_vec_table_itcm;
  SCB->VTOR = ((unsigned int)p_src & SCB_VTOR_TBLOFF_Msk);

  main();

  /* Infinite loop */
  while (1)
    ;
}

BOOTFUNC
void Reset_Handler(void)
{
  // Init stack pointer, needed for SRAM startup (debug)
  asm volatile("ldr   sp, =_estack");

  boot();
}

/**
 * \brief Default interrupt handler for unused IRQs.
 */

unsigned int icsr;

BOOTFUNC
void Dummy_Handler(void)
{
  icsr = SCB->ICSR;
  while (1)
  {
  }
}
