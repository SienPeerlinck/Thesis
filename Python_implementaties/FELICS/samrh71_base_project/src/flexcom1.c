#include "sam.h"

#include "flexcom1.h"

// FreeRTOS includes
#include "FreeRTOS.h"
#include "queue.h"

#define USART1_TX_Q_LEN 32
#define USART1_RX_Q_LEN 256

unsigned int cntr_tx_ok = 0;
unsigned int cntr_tx_q_empty = 0;
unsigned int cntr_rx_q_full = 0;
unsigned int cntr_rx_ok = 0;

static QueueHandle_t usart1_tx_queue;
static QueueHandle_t usart1_rx_queue;

unsigned char usart1_rx_overrun_error = 0;

void init_usart1(void)
{
    usart1_tx_queue = xQueueCreate(USART1_TX_Q_LEN, sizeof(char));
    usart1_rx_queue = xQueueCreate(USART1_RX_Q_LEN, sizeof(char));

    // I/O line PF29 & PF30 assigned to peripheral A (FLEXCOM1)
    PIO_REGS->PIO_GROUP[5].PIO_MSKR = PIO_MSKR_MSK29(1) | PIO_MSKR_MSK30(1);
    PIO_REGS->PIO_GROUP[5].PIO_CFGR = PIO_CFGR_FUNC_PERIPH_A;

    // Enable Peripheral Clock for FLEXCOM1 (id=8), GCLK source = MAIN_CLK
    //PMC_REGS->PMC_PCR = PMC_PCR_EN(1) | PMC_PCR_GCLKCSS_MAIN_CLK | PMC_PCR_CMD(1) | PMC_PCR_PID(8);
    // Enable GCLK for FLEXCOM1 (id=8)
    //PMC_REGS->PMC_PCR = PMC_PCR_EN(1) | PMC_PCR_GCLKCSS_MAIN_CLK | PMC_PCR_CMD(1) | PMC_PCR_PID(8) | PMC_PCR_GCLKEN(1);

    // Enable Peripheral Clock & GCLK for FLEXCOM1 (id=8), GCLK source = PLLACK, DIV=3+1 ==> GCLK = 100 / 4 = 25MHz
    PMC_REGS->PMC_PCR = PMC_PCR_EN(1) | PMC_PCR_GCLKCSS_PLLA_CLK | PMC_PCR_CMD(1) | PMC_PCR_PID(8) | PMC_PCR_GCLKEN(1) | PMC_PCR_GCLKDIV(3);

    // Baud Rate Generator Clock = GCLK
    FLEXCOM1_REGS->FLEX_US_MR = FLEX_US_MR_USCLKS_GCLK;

    // 8 bits, NO parity
    FLEXCOM1_REGS->FLEX_US_MR |= FLEX_US_MR_CHRL_8_BIT | FLEX_US_MR_PAR_NO;

    // Baud Rate = GCLK / (16 * (CD + FP/8))  = 25MHZ / (16 * (13 + 4/8)) = 115740 (≃ 115200) Baud
    FLEXCOM1_REGS->FLEX_US_BRGR = FLEX_US_BRGR_CD(13) | FLEX_US_BRGR_FP(4);

    // Enable Tx/Rx FIFOs
    FLEXCOM1_REGS->FLEX_US_CR = FLEX_US_CR_FIFOEN(0);

    // Enable Interrupts
    NVIC_SetPriority(FLEXCOM1_IRQn, 5);
    NVIC_EnableIRQ(FLEXCOM1_IRQn);
    FLEXCOM1_REGS->FLEX_US_IER = FLEX_US_IER_RXRDY_Msk;

    // Enable Transmitter & Receiver
    FLEXCOM1_REGS->FLEX_US_CR |= FLEX_US_CR_TXEN(1) | FLEX_US_CR_RXEN(1);
}

int usart1_getchar(TickType_t timeout)
{
    char c;
    if (usart1_rx_overrun_error)
        return -1;
    if (xQueueReceive(usart1_rx_queue, &c, timeout))
        return (int)c;
    else
        return -1;
}

void usart1_putchar(char c, TickType_t timeout)
{
    xQueueSendToBack(usart1_tx_queue, &c, timeout);
    FLEXCOM1_REGS->FLEX_US_IER = FLEX_US_IER_TXEMPTY_Msk;
}

void FLEXCOM1_Handler(void)
{
    char c, dummy_c;

    if (FLEXCOM1_REGS->FLEX_US_CSR & FLEX_US_CSR_TXRDY_Msk)
    {
        if (xQueueReceiveFromISR(usart1_tx_queue, &c, NULL)) {
            FLEXCOM1_REGS->FLEX_US_THR = FLEX_US_THR_TXCHR(c);
            cntr_tx_ok++;
        }
        else {
            // When queue empty, disable TXEMPTY interrupt
            FLEXCOM1_REGS->FLEX_US_IDR = FLEX_US_IDR_TXEMPTY_Msk;
            cntr_tx_q_empty++;
        }
    }

    if (FLEXCOM1_REGS->FLEX_US_CSR & FLEX_US_CSR_RXRDY_Msk)
    {
        c = FLEXCOM1_REGS->FLEX_US_RHR & 0xff;
        if (xQueueSendToBackFromISR(usart1_rx_queue, &c, NULL) == errQUEUE_FULL)
        {
            usart1_rx_overrun_error = 1;
            xQueueReceiveFromISR(usart1_rx_queue, &dummy_c, NULL);
            xQueueSendToBackFromISR(usart1_rx_queue, &c, NULL);
            cntr_rx_q_full++;
        } else cntr_rx_ok++;
    }

    /*

    if (USART1->US_CSR.bit.RXRDY) {
      c = USART1->US_RHR.reg;
      if (xQueueSendToBackFromISR(usart1_rx_queue, &c, NULL) == errQUEUE_FULL) {
        usart1_rx_overrun_error = 1;
        xQueueReceiveFromISR(usart1_rx_queue, &dummy_c, NULL);
        xQueueSendToBackFromISR(usart1_rx_queue, &c, NULL);
      }
    }
    //NVIC_ClearPendingIRQ(USART1_IRQn);
    */
}