#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/* Ensure stdint is only used by the compiler, and not the assembler. */
#if defined(__ICCARM__) || defined(__CC_ARM) || defined(__GNUC__)
//#include <stdint.h>
extern uint32_t SystemCoreClock;
// void assert_triggered(const char *file, uint32_t line);
#endif

#define configUSE_PREEMPTION 1
//#define configUSE_TIME_SLICING                   1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 1
#define configSUPPORT_STATIC_ALLOCATION 0
#define configSUPPORT_DYNAMIC_ALLOCATION 1
#define configUSE_IDLE_HOOK 0
#define configUSE_TICK_HOOK 0
#define configCPU_CLOCK_HZ (SystemCoreClock)
//#define configSYSTICK_CLOCK_HZ                   ( SystemCoreClock )
#define configTICK_RATE_HZ ((TickType_t)100)

#define configMAX_PRIORITIES (5)
#define configMINIMAL_STACK_SIZE ((uint16_t)64)
//#define configTOTAL_HEAP_SIZE ((size_t)0x10000)  // For heap_4.c
//#define configGENERATE_RUN_TIME_STATS 1
//#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS 1
//#define portGET_RUN_TIME_COUNTER_VALUE 1
//#define configUSE_QUEUE_SETS 1

#define configUSE_16_BIT_TICKS 0

// Needed by vTaskList
#define configUSE_TRACE_FACILITY 1
#define configUSE_STATS_FORMATTING_FUNCTIONS 1

/* Set the following definitions to 1 to include the API function, or zero
to exclude the API function. */
#define INCLUDE_vTaskDelay 1
#define INCLUDE_vTaskSuspend 1

/* Cortex-M specific definitions. */
#ifdef __NVIC_PRIO_BITS
/* __BVIC_PRIO_BITS will be specified when CMSIS is being used. */
#define configPRIO_BITS __NVIC_PRIO_BITS
#else
#define configPRIO_BITS 3
#endif
/* The lowest interrupt priority that can be used in a call to a "set priority"
function. */
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY 15

/* The highest interrupt priority that can be used by any interrupt service
routine that makes calls to interrupt safe FreeRTOS API functions.  DO NOT CALL
INTERRUPT SAFE FREERTOS API FUNCTIONS FROM ANY INTERRUPT THAT HAS A HIGHER
PRIORITY THAN THIS! (higher priorities are lower numeric values. */
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5

/* Interrupt priorities used by the kernel port layer itself.  These are generic
to all Cortex-M ports, and do not rely on any particular library functions. */
#define configKERNEL_INTERRUPT_PRIORITY (configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))
/* !!!! configMAX_SYSCALL_INTERRUPT_PRIORITY must not be set to zero !!!!
See http://www.FreeRTOS.org/RTOS-Cortex-M3-M4.html. */
#define configMAX_SYSCALL_INTERRUPT_PRIORITY (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))

/* Normal assert() semantics without relying on the provision of an assert.h
header file. */
#define configASSERT(x)           \
    if ((x) == 0)                 \
    {                             \
        taskDISABLE_INTERRUPTS(); \
        for (;;)                  \
            ;                     \
    }

/* Definitions that map the FreeRTOS port interrupt handlers to their CMSIS
standard names. */
#define vPortSVCHandler SVCall_Handler
#define xPortPendSVHandler PendSV_Handler
#define xPortSysTickHandler SysTick_Handler

/* Low Power Support */
#define configUSE_TICKLESS_IDLE 1
#define configEXPECTED_IDLE_TIME_BEFORE_SLEEP 5

/* Enable Run Time Statistics */
extern void init_timer0(void); /* defined in timer1ch0.c */
extern unsigned int get_timer0_cnt(void);

#define configGENERATE_RUN_TIME_STATS 1
#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS() init_timer0()
#define portGET_RUN_TIME_COUNTER_VALUE() get_timer0_cnt()

#endif