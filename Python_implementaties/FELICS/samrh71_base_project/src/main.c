/*
 * main.c
 *
 * Created: 23/3/2020
 * Author : Roland Clairquin
 */

#include "sam.h"

// FreeRTOS includes
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "console.h"
#include "tc_tm.h"
#include "spacewire.h"
#include "spw_task.h"
#include "opt/printf-stdarg.h"
#include "timer0.h"
#include "ext_flash.h"
#include "flexcom1.h"
#include "obt.h"
#include "flexcom3.h"
#include "fpga_contents.h"

extern char _sheap;
extern char _eheap;
extern char _estack;
extern char _vec_table;
extern char _vec_table_itcm;
extern char _erelocate_i;
extern char _etext;
extern char _eflash;
extern char boot_status;
extern unsigned int boot_pc;
extern unsigned int reset_status;
extern void boot(void);


//uint32_t SystemCoreClock = 12000000; // Modified in init_pmc()

extern char rcv_buf_static[];

typedef struct
{
  unsigned random_seed : 8;
  unsigned tmlen : 24;
} tc_t;

static QueueHandle_t spw_trig_queue;

static void init_leds(void)
{
  // Enable Peripheral Clock for PIOB (id=11)
  PMC_REGS->PMC_PCR = PMC_PCR_EN(1) | PMC_PCR_CMD(1) | PMC_PCR_PID(11);

  // PB19 (LED0) & PB23 (LED1) are outputs
  PIO_REGS->PIO_GROUP[1].PIO_MSKR = PIO_MSKR_MSK19(1) | PIO_MSKR_MSK23(1);
  PIO_REGS->PIO_GROUP[1].PIO_CFGR = PIO_CFGR_DIR_OUTPUT;

  // Enable Peripheral Clock for PIOF (id=34)
  PMC_REGS->PMC_PCR = PMC_PCR_EN(1) | PMC_PCR_CMD(1) | PMC_PCR_PID(34);

  // PF19 (LED2) & PF20 (LED3) are outputs
  PIO_REGS->PIO_GROUP[5].PIO_MSKR = PIO_MSKR_MSK19(1) | PIO_MSKR_MSK20(1);
  PIO_REGS->PIO_GROUP[5].PIO_CFGR = PIO_CFGR_DIR_OUTPUT;
}

static void init_reset_pin(void) {
  // Enable Peripheral Clock for PIOA (id=10)
  PMC_REGS->PMC_PCR = PMC_PCR_EN(1) | PMC_PCR_CMD(1) | PMC_PCR_PID(10);

  // Reset Pin PA13: Output, open drain, value=1
  PIO_REGS->PIO_GROUP[0].PIO_SODR = PIO_SODR_P13(1);
  PIO_REGS->PIO_GROUP[0].PIO_MSKR = PIO_MSKR_MSK13(1);
  PIO_REGS->PIO_GROUP[0].PIO_CFGR = PIO_CFGR_DIR_OUTPUT | PIO_CFGR_OPD_ENABLED | PIO_CFGR_DRVSTR_OUT_48m;
}

static void init_xtal_slow_clock(void) {
  // From datasheet
  // 20.4.2 Slow Clock Generator

  // External slow clock start-up time is 1 slow RC period.
  // SUPC_REGS->SUPC_MR = SUPC_MR_KEY(0xA5) | SUPC_MR_FXTALSTUP(0);

  // Switch 'Time Domain Slow Clock' to External Oscillator
  SUPC_REGS->SUPC_CR = SUPC_CR_KEY(0xA5) | SUPC_CR_TDXTALSEL(1);

  // Wait for the switch to complete
  while ((SUPC_REGS->SUPC_SR & SUPC_SR_TDOSCSEL_Msk) == 0);
}

static void init_freertos(void)
{
  // Initialise Heap 5 before using any kernel objects
  HeapRegion_t xHeapRegions[] = {
      {(unsigned char *)&_sheap,
       ((const char *)&_eheap - (const char *)&_sheap)},
      {NULL, 0} /* Marks the end of the array. */};
  vPortDefineHeapRegions(xHeapRegions);
}

static void blinking_led1_task(void *pvParameters)
{
  while (1)
  {
    // Turn LED1 On
    PIO_REGS->PIO_GROUP[1].PIO_SODR = PIO_SODR_P19(1);
    vTaskDelay(100);

    // Turn LED1 Off
    PIO_REGS->PIO_GROUP[1].PIO_CODR = PIO_CODR_P19(1);
    vTaskDelay(100);
  }
}

static void show_info(void)
{
  printf("\nVenSpec-H Flight Software\n");
  printf("Compiled with gcc %s  -  %s %s\n", __VERSION__, __DATE__, __TIME__);
  printf("FreeRTOS %s\n", tskKERNEL_VERSION_NUMBER);
  if (boot_status == 1)
    printf("Boot from External Flash (boot_status=%02x)\n", boot_status);
  else
    printf("Boot from Embedded Flash (boot_status=%02x)\n", boot_status);
  
  printf("reset_status: %08x\n", reset_status);
  if ((reset_status & RSTC_SR_RSTTYP_Msk) == RSTC_SR_RSTTYP_WDT_RST)
    printf("!!! Watchdog Reset !!!\n");
}

static void show_task_stats(void)
{
  TaskStatus_t *task_array;
  volatile UBaseType_t task_array_size;
  unsigned long total_runtime;

  /* Take a snapshot of the number of tasks in case it changes while this
  function is executing. */
  task_array_size = uxTaskGetNumberOfTasks();

  /* Allocate a TaskStatus_t structure for each task.  An array could be
  allocated statically at compile time. */
  task_array = pvPortMalloc(task_array_size * sizeof(TaskStatus_t));

  if (task_array != NULL)
  {
    /* Generate raw status information about each task. */
    task_array_size = uxTaskGetSystemState(task_array,
                                           task_array_size,
                                           &total_runtime);

    /* For each populated position in the pxTaskStatusArray array,
    format the raw data as human readable ASCII data. */
    for (unsigned int i = 0; i < task_array_size; i++)
    {
      printf("%s\n", task_array[i].pcTaskName);
      printf("  xTaskNumber          %d\n", task_array[i].xTaskNumber);
      printf("  eCurrentState        %d\n", task_array[i].eCurrentState);
      printf("  uxCurrentPriority    %d\n", task_array[i].uxCurrentPriority);
      printf("  uxBasePriority       %d\n", task_array[i].uxBasePriority);
      printf("  ulRunTimeCounter     %d\n", task_array[i].ulRunTimeCounter);
      printf("  pxStackBase          Ox%x\n", task_array[i].pxStackBase);
      printf("  usStackHighWaterMark %d\n", task_array[i].usStackHighWaterMark);
    }

    /* The array is no longer needed, free the memory it consumes. */
    vPortFree(task_array);
  }
  else
  {
    printf("pvPortMalloc Error(%d) !\n", task_array_size * sizeof(TaskStatus_t));
  }
}

static void show_heap_stats(void)
{
  HeapStats_t xHeapStats;
  vPortGetHeapStats(&xHeapStats);
  printf("Heap Status\n");
  printf(" xSizeOfLargestFreeBlockInBytes   = %d\n", xHeapStats.xSizeOfLargestFreeBlockInBytes);
  printf(" xSizeOfSmallestFreeBlockInBytes  = %d\n", xHeapStats.xSizeOfSmallestFreeBlockInBytes);
  printf(" xNumberOfFreeBlocks              = %d\n", xHeapStats.xNumberOfFreeBlocks);
  printf(" xMinimumEverFreeBytesRemaining   = %d\n", xHeapStats.xMinimumEverFreeBytesRemaining);
  printf(" xNumberOfSuccessfulAllocations   = %d\n", xHeapStats.xNumberOfSuccessfulAllocations);
  printf(" xNumberOfSuccessfulFrees         = %d\n", xHeapStats.xNumberOfSuccessfulFrees);
}

static char __attribute__((aligned(32))) buf[20];

static void console_task(void *pvParameters)
{
  char c;
  char trigger;
  unsigned int tcmecc_status;
  Obt_t obt;
  int i=1;

  printf("\n\n!!! Console task started !!!\n");
  show_info();

  while (1)
  {
    c = (char)getchar_with_timeout(10);
    switch (c)
    {
    case 'a':
      printf("\nPMC Clock Generator Main Oscillator Register (CKGR_MOR): %08x\n",
             PMC_REGS->CKGR_MOR);
      printf("RTC_TIMR: %08x\n", RTC_REGS->RTC_TIMR);
      printf("RTC_CALR: %08x\n", RTC_REGS->RTC_CALR);
      printf("RTC_MSR:  %08x\n", RTC_REGS->RTC_MSR);
      get_obt(&obt);
      print_obt(&obt);
      printf("\n");
      break;

    case 'i':
      show_info();
      break;

    case 't':
      printf("\n== Tasks Status ==\n");
      show_task_stats();
      break;

    case 's':
      printf("\n== Spacewire Status ==\n");
      print_spw_status();
      break;

    case 'g':
      // Start spacewire test
      xQueueSendToBack(spw_trig_queue, &trigger, NULL);
      break;

    case 'w':
      copy_embedded_flash_to_ext_flash();
      break;

    case 'r':
      // Hard Reset - GPIO PA13 -> NRST
      PIO_REGS->PIO_GROUP[0].PIO_CODR = PIO_CODR_P13(1);
      break;

    case 'z':
      // Soft Reset
      RSTC_REGS->RSTC_CR = RSTC_CR_KEY_PASSWD | RSTC_CR_PROCRST(1);
      break;

    /*
    case 'f':
      printf("\n== UART (FlexCom1) Status ==\n");
      printf("cntr_tx_ok      %d\n", cntr_tx_ok);
      printf("cntr_tx_q_empty %d\n", cntr_tx_q_empty);
      printf("cntr_rx_ok      %d\n", cntr_rx_ok);
      printf("cntr_rx_q_full  %d\n", cntr_rx_q_full);
      break;
    */
    case 'f':
      // Program FPGA
      //printf("0 FLEX_SPI_FLR %08x\n", FLEXCOM3_REGS->FLEX_SPI_FLR);

      // Activate FPGA RST-
      PIO_REGS->PIO_GROUP[0].PIO_CODR = PIO_CODR_P21(1);
      vTaskDelay(5);
      spi3_transmit(fpga_contents, FPGA_CONTENTS_N);
      //spi3_transmit(fpga_contents, 20);
      vTaskDelay(1);
      // Deactivate FPGA RST-
      PIO_REGS->PIO_GROUP[0].PIO_SODR = PIO_SODR_P21(1);

      //printf("1 FLEX_SPI_FLR %08x\n", FLEXCOM3_REGS->FLEX_SPI_FLR);
      //printf("  spi3_tx_remaining_bytes %d\n", spi3_tx_remaining_bytes());
      
      break;

    case 'm': // Memory Info
      printf("\n== Memory Status ==\n");
      tcmecc_status = TCMECC_REGS->TCMECC_SR;
      printf("TCM ECC Status     : %08x\n", tcmecc_status);
      if (tcmecc_status & 0x00000303) {
        printf("  TCMHECC Fail Address: %08x\n", TCMECC_REGS->TCMECC_FAILAR);
        printf("  TCMHECC Fail Data   : %08x\n", TCMECC_REGS->TCMECC_FAILARD);
      }
      printf("FlexRAM ECC Status : %08x\n", FLEXRAMECC_REGS->FLEXRAMECC_SR);
      printf("HEFC Power Status  : %08x\n", HEFC_REGS->HEFC_FPMR);
      show_heap_stats();
      break;

    case 255: // Timeout
      break;

    default:
      printf("Unknown command: %d\n\n", c);
      break;

      /*
      case 'g':
        printf("\n");
        sprintf(buf, "Hellp");
        entry = send_buf(buf, 5);
        vTaskDelay(100);
        //vPortFree(entry);
        break;

      case 'r':
        printf("\n");
        init_rcv();
        break;
      */
    }
  }
}

static void watchdog_task(void *pvParameters)
{
  // Watchdog Restart after boot
  WDT_REGS->WDT_CR = WDT_CR_KEY(0xA5) | WDT_CR_WDRSTT(1);

  // Config
  // Watchdog counter value: WDV = 0x280 ==> 2.5 s
  // Watchdog delta value:   WDD = 0x100 ==> 1 s
  WDT_REGS->WDT_MR = WDT_MR_WDV(0x280) | WDT_MR_WDD(0x100) | WDT_MR_WDIDLEHLT(1) |
                     WDT_MR_WDDBGHLT(1) | WDT_MR_WDRSTEN(1);
  
  // Config done ==> Lock WDT_MR register
  WDT_REGS->WDT_CR = WDT_CR_KEY(0xA5) | WDT_CR_LOCKMR(1);

  while (1) {
    // Wait for 2 seconds
    vTaskDelay(200);

    // Test that other tasks are still alive
    // FixMe

    // Watchdog Restart
    WDT_REGS->WDT_CR = WDT_CR_KEY(0xA5) | WDT_CR_WDRSTT(1);
  }
}

static const char RMAP_CRC_TABLE[] = {
    0x00, 0x91, 0xe3, 0x72, 0x07, 0x96, 0xe4, 0x75, // 0
    0x0e, 0x9f, 0xed, 0x7c, 0x09, 0x98, 0xea, 0x7b, // 1
    0x1c, 0x8d, 0xff, 0x6e, 0x1b, 0x8a, 0xf8, 0x69, // 2
    0x12, 0x83, 0xf1, 0x60, 0x15, 0x84, 0xf6, 0x67, // 3
    0x38, 0xa9, 0xdb, 0x4a, 0x3f, 0xae, 0xdc, 0x4d, // 4
    0x36, 0xa7, 0xd5, 0x44, 0x31, 0xa0, 0xd2, 0x43, // 5
    0x24, 0xb5, 0xc7, 0x56, 0x23, 0xb2, 0xc0, 0x51, // 6
    0x2a, 0xbb, 0xc9, 0x58, 0x2d, 0xbc, 0xce, 0x5f, // 7
    0x70, 0xe1, 0x93, 0x02, 0x77, 0xe6, 0x94, 0x05, // 8
    0x7e, 0xef, 0x9d, 0x0c, 0x79, 0xe8, 0x9a, 0x0b, // 9
    0x6c, 0xfd, 0x8f, 0x1e, 0x6b, 0xfa, 0x88, 0x19, // 10
    0x62, 0xf3, 0x81, 0x10, 0x65, 0xf4, 0x86, 0x17, // 11
    0x48, 0xd9, 0xab, 0x3a, 0x4f, 0xde, 0xac, 0x3d, // 12
    0x46, 0xd7, 0xa5, 0x34, 0x41, 0xd0, 0xa2, 0x33, // 13
    0x54, 0xc5, 0xb7, 0x26, 0x53, 0xc2, 0xb0, 0x21, // 14
    0x5a, 0xcb, 0xb9, 0x28, 0x5d, 0xcc, 0xbe, 0x2f, // 15
    0xe0, 0x71, 0x03, 0x92, 0xe7, 0x76, 0x04, 0x95, // 16
    0xee, 0x7f, 0x0d, 0x9c, 0xe9, 0x78, 0x0a, 0x9b, // 17
    0xfc, 0x6d, 0x1f, 0x8e, 0xfb, 0x6a, 0x18, 0x89, // 18
    0xf2, 0x63, 0x11, 0x80, 0xf5, 0x64, 0x16, 0x87, // 19
    0xd8, 0x49, 0x3b, 0xaa, 0xdf, 0x4e, 0x3c, 0xad, // 20
    0xd6, 0x47, 0x35, 0xa4, 0xd1, 0x40, 0x32, 0xa3, // 21
    0xc4, 0x55, 0x27, 0xb6, 0xc3, 0x52, 0x20, 0xb1, // 22
    0xca, 0x5b, 0x29, 0xb8, 0xcd, 0x5c, 0x2e, 0xbf, // 23
    0x90, 0x01, 0x73, 0xe2, 0x97, 0x06, 0x74, 0xe5, // 24
    0x9e, 0x0f, 0x7d, 0xec, 0x99, 0x08, 0x7a, 0xeb, // 25
    0x8c, 0x1d, 0x6f, 0xfe, 0x8b, 0x1a, 0x68, 0xf9, // 26
    0x82, 0x13, 0x61, 0xf0, 0x85, 0x14, 0x66, 0xf7, // 27
    0xa8, 0x39, 0x4b, 0xda, 0xaf, 0x3e, 0x4c, 0xdd, // 28
    0xa6, 0x37, 0x45, 0xd4, 0xa1, 0x30, 0x42, 0xd3, // 29
    0xb4, 0x25, 0x57, 0xc6, 0xb3, 0x22, 0x50, 0xc1, // 30
    0xba, 0x2b, 0x59, 0xc8, 0xbd, 0x2c, 0x5e, 0xcf  // 31
};

char calculate_crc(char *data, int n)
{
  char crc;
  crc = 0x00;
  for (int i = 0; i < n; i++)
  {
    crc = RMAP_CRC_TABLE[(crc ^ *data++) & 0xff];
  }
  return crc;
}

char calculate_crc_slow(char *data, int n)
{
  char crc, x;
  crc = 0x00;
  for (int i = 0; i < n; i++)
  {
    x = *data++;
    for (int j = 0; j < 8; j++)
    {
      if ((crc ^ x) & 1)
        crc = (crc >> 1) ^ 0xe0;
      else
        crc = crc >> 1;
      x >>= 1;
    }
  }
  return crc;
}

char calculate_crc_fast(char *data, int n)
{
  char crc, x, x0, x1, x2, x3, x4;
  crc = 0x00;
  for (int i = 0; i < n; i++)
  {
    x = *data++;
    x0 = x ^ crc;
    x1 = x0 >> 1 | x0 << 7;
    x2 = x1 >> 1 | x1 << 7;
    x3 = x2 >> 1 | x2 << 7;
    x4 = x3 >> 1 | x3 << 7;
    crc = x0 ^ x1 ^ (x2 & 0xbf) ^ (x3 & 0x40) ^ (x4 & 0x30);
  }
  return crc;
}

static void spw_loop_task(void *pvParameters)
{
  send_desc_t *send_desc;
  rcv_desc_t *rcv_desc;
  char *tm_buf, *header;
  int n, h;
  int n_loop = 100;
  int tm_len = 276480;
  // int tm_len = 10000;
  char err;
  char crc;
  char trigger;
  unsigned int t0, t1;
  unsigned int n_errors;
  tc_t *tc;

  vTaskDelay(100);
  spw_start();
  while (1)
  {
    xQueueReceive(spw_trig_queue, &trigger, -1);
    printf("\nSpacewire Test\n");
    n_errors = 0;
    t0 = get_timer0_cnt();
    for (int j = 0; j < n_loop; j++)
    {
      while (err = spw_get_send_desc(&send_desc))
        vTaskDelay(1);
      while (err = spw_get_tx_buf(&tc))
        vTaskDelay(1);
      while (err = spw_get_tx_header(&header))
        vTaskDelay(1);

      tc->tmlen = tm_len + 2;
      tc->random_seed = j & 0xff;

      h = 0;
      for (int i = 0; i < h; i++)
        header[i] = 42;

      send_desc->str.daddr = tc;
      send_desc->str.dsize = sizeof(tc_t);
      send_desc->str.haddr = header;
      send_desc->str.hsize = h;
      send_desc->str.rb1 = 1;
      // send_desc->str.rb2 = 9;
      send_desc->str.rsize = 1;
      send_desc->str.dcrc = 1;
      // send_desc->str.hcrc = 1;
      err = spw_send(send_desc);

      err = spw_wait_send_done(&send_desc, -1);
      err = spw_release_tx_buf(send_desc->str.daddr);
      err = spw_release_tx_header(send_desc->str.haddr);
      spw_release_send_desc(send_desc);

      err = spw_receive(&rcv_desc, -1);
      n = rcv_desc->str.dsize;
      tm_buf = rcv_desc->str.daddr;

      
      for (int i = 0; i < 10; i++)
        printf(" %02x", tm_buf[i]);
      printf("   rcv_desc=%08x  n=%d  crc=%02x\n",rcv_desc, n, rcv_desc->str.crc);
      
      if (rcv_desc->str.crc)
        n_errors++;

      spw_release_rcv_desc(rcv_desc);
      // vTaskDelay(20);
    }
    t1 = get_timer0_cnt();
    printf("Done: %d * %d bytes in %d ms    %d crc errors\n", n_loop, n, t1 - t0, n_errors);
    
  }
}

static void start_tasks(void)
{
  xTaskCreate(blinking_led1_task, "blnk", 64, NULL, 1, NULL);
  xTaskCreate(watchdog_task, "wdog", 64, NULL, 1, NULL);
  xTaskCreate(console_task, "cons", 256, NULL, 1, NULL);
  xTaskCreate(tc_task, "tcom", 256, NULL, 1, NULL);
  xTaskCreate(spw_task, "spwt", 256, NULL, 2, NULL);
  //xTaskCreate(meas_task, "meas", 256, NULL, 1, NULL);
  // xTaskCreate(spw_send_loop_task, "spw_send_loop_task", 256, NULL, 2, NULL);
  //xTaskCreate(spw_release_task, "spwr", 128, NULL, 3, NULL);
  //xTaskCreate(spw_tc_task, "spwc", 128, NULL, 1, NULL);
  //xTaskCreate(spw_loop_task, "spwr", 256, NULL, 2, NULL);

  vTaskStartScheduler();
}

void init_matrix(void)
{
  MATRIX0_REGS->MATRIX_PASSR[0] = 0x00000FFF;
  MATRIX0_REGS->MATRIX_PSR[0] = 0x07070707;
  MATRIX0_REGS->MATRIX_PASSR[1] = 0x00000FFF;
  MATRIX0_REGS->MATRIX_PSR[1] = 0x07070707;
  MATRIX0_REGS->MATRIX_PASSR[6] = 0x00000FFF;
  MATRIX0_REGS->MATRIX_PSR[6] = 0x07070707;
  MATRIX0_REGS->MATRIX_PASSR[7] = 0x00000FFF;
  MATRIX0_REGS->MATRIX_PSR[7] = 0x07070707;
}

int main(void)
{
  // SCB_InvalidateICache();
  // SCB_InvalidateDCache();
  // SCB_EnableICache();
  // SCB_EnableDCache();

  // Switch off Embedded Flash
  HEFC_REGS->HEFC_FPMR &= ~HEFC_FPMR_PWS_EN_Msk;

  init_reset_pin();
  init_leds();
  init_xtal_slow_clock();
  // PIO_REGS->PIO_GROUP[1].PIO_CODR = PIO_CODR_P23(1);
  init_freertos();
  init_console();
  init_telecommand();
  spw_init();
  init_matrix();
  init_ext_flash();
  init_spi3();
  spw_trig_queue = xQueueCreate(10, sizeof(char));
  start_tasks();

  while (1)
  {
  }
}

// For debugging
void HardFault_Handler(void)
{
  volatile int hf_status = SCB->HFSR;
  while (1)
  {
  }
}
