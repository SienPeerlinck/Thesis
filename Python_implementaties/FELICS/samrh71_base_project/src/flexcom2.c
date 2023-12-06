#include "sam.h"

#include "flexcom2.h"

// FreeRTOS includes
#include "FreeRTOS.h"
#include "queue.h"

#define USART2_TX_Q_LEN 32
#define USART2_RX_Q_LEN 256

static QueueHandle_t usart2_tx_queue;
static QueueHandle_t usart2_rx_queue;

unsigned char usart2_rx_overrun_error = 0;

void init_usart2(void)
{
    usart2_tx_queue = xQueueCreate(USART2_TX_Q_LEN, sizeof(char));
    usart2_rx_queue = xQueueCreate(USART2_RX_Q_LEN, sizeof(char));

    // I/O line PA2 & PA6 assigned to peripheral A (FLEXCOM2)
    PIO_REGS->PIO_GROUP[0].PIO_MSKR = PIO_MSKR_MSK2(1) | PIO_MSKR_MSK6(1);
    PIO_REGS->PIO_GROUP[0].PIO_CFGR = PIO_CFGR_FUNC_PERIPH_A;

    // Enable Peripheral Clock & GCLK for FLEXCOM2 (id=13), GCLK source = PLLACK, DIV=3+1 ==> GCLK = 100 / 4 = 25MHz
    PMC_REGS->PMC_PCR = PMC_PCR_EN(1) | PMC_PCR_GCLKCSS_PLLA_CLK | PMC_PCR_CMD(1) | PMC_PCR_PID(13) | PMC_PCR_GCLKEN(1) | PMC_PCR_GCLKDIV(3);

    // Baud Rate Generator Clock = GCLK
    FLEXCOM2_REGS->FLEX_US_MR = FLEX_US_MR_USCLKS_GCLK;

    // 8 bits, NO parity
    FLEXCOM2_REGS->FLEX_US_MR |= FLEX_US_MR_CHRL_8_BIT | FLEX_US_MR_PAR_NO;

    // Baud Rate = GCLK / (16 * (CD + FP/8))  = 25MHZ / (16 * (13 + 4/8)) = 115740 (≃ 115200) Baud
    FLEXCOM2_REGS->FLEX_US_BRGR = FLEX_US_BRGR_CD(13) | FLEX_US_BRGR_FP(4);

    // Enable Tx/Rx FIFOs
    FLEXCOM2_REGS->FLEX_US_CR = FLEX_US_CR_FIFOEN(0);

    // Enable Interrupts
    NVIC_SetPriority(FLEXCOM2_IRQn, 5);
    NVIC_EnableIRQ(FLEXCOM2_IRQn);
    FLEXCOM2_REGS->FLEX_US_IER = FLEX_US_IER_RXRDY_Msk;

    // Enable Transmitter & Receiver
    FLEXCOM2_REGS->FLEX_US_CR |= FLEX_US_CR_TXEN(1) | FLEX_US_CR_RXEN(1);
}

int usart2_getchar(TickType_t timeout)
{
    char c;
    if (usart2_rx_overrun_error)
        return -1;
    if (xQueueReceive(usart2_rx_queue, &c, timeout))
        return (int)c;
    else
        return -1;
}

void usart2_putchar(char c, TickType_t timeout)
{
    xQueueSendToBack(usart2_tx_queue, &c, timeout);
    FLEXCOM2_REGS->FLEX_US_IER = FLEX_US_IER_TXEMPTY_Msk;
}

void FLEXCOM2_Handler(void)
{
    char c, dummy_c;

    if (FLEXCOM2_REGS->FLEX_US_CSR & FLEX_US_CSR_TXRDY_Msk)
    {
        if (xQueueReceiveFromISR(usart2_tx_queue, &c, NULL))
            FLEXCOM2_REGS->FLEX_US_THR = FLEX_US_THR_TXCHR(c);
        else
            // When queue empty, disable TXEMPTY interrupt
            FLEXCOM2_REGS->FLEX_US_IDR = FLEX_US_IDR_TXEMPTY_Msk;
    }

    if (FLEXCOM2_REGS->FLEX_US_CSR & FLEX_US_CSR_RXRDY_Msk)
    {
        c = FLEXCOM2_REGS->FLEX_US_RHR & 0xff;
        if (xQueueSendToBackFromISR(usart2_rx_queue, &c, NULL) == errQUEUE_FULL)
        {
            usart2_rx_overrun_error = 1;
            xQueueReceiveFromISR(usart2_rx_queue, &dummy_c, NULL);
            xQueueSendToBackFromISR(usart2_rx_queue, &c, NULL);
        }
    }
}