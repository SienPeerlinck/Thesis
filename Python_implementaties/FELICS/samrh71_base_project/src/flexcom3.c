#include "sam.h"

#include "flexcom3.h"

static char* tx_ptr;
static int tx_cntr = 0;

void init_spi3(void)
{
    // FixMe to be moved to fpga.c (?) ----------------------------------------
    // FPGA RST-   PA21
    // Enable Peripheral Clock for PIOA (id=10)
    PMC_REGS->PMC_PCR = PMC_PCR_EN(1) | PMC_PCR_CMD(1) | PMC_PCR_PID(10);

    // PA21 as Output, open-drain
    PIO_REGS->PIO_GROUP[0].PIO_MSKR = PIO_MSKR_MSK21(1);
    PIO_REGS->PIO_GROUP[0].PIO_CFGR = PIO_CFGR_DIR_OUTPUT | PIO_CFGR_OPD_ENABLED;

    // Activate FPGA RST-
    PIO_REGS->PIO_GROUP[0].PIO_CODR = PIO_CODR_P21(1);

    //--------------------------------------------------------------------------

    // I/O line PA19, PA20, PA23 & PA24 assigned to peripheral A (FLEXCOM3)
    PIO_REGS->PIO_GROUP[0].PIO_MSKR = PIO_MSKR_MSK19(1) | PIO_MSKR_MSK20(1) |\
                                      PIO_MSKR_MSK23(1) | PIO_MSKR_MSK24(1);
    PIO_REGS->PIO_GROUP[0].PIO_CFGR = PIO_CFGR_FUNC_PERIPH_A;

    // Enable Peripheral Clock & GCLK for FLEXCOM3 (id=14), GCLK source = PLLACK, DIV=3+1 ==> GCLK = 100 / 4 = 25MHz
    PMC_REGS->PMC_PCR = PMC_PCR_EN(1) | PMC_PCR_GCLKCSS_PLLA_CLK | PMC_PCR_CMD(1) | PMC_PCR_PID(14) | PMC_PCR_GCLKEN(1) | PMC_PCR_GCLKDIV(3);

    // SPI operating mode is selected.
    FLEXCOM3_REGS->FLEX_MR = FLEX_MR_OPMODE_SPI;
    
    // ! SPI is in Slave mode by default

    // 8 bits, change on leading edge capture on following edge, SPCK inactive @ 0
    FLEXCOM3_REGS->FLEX_SPI_CSR[0] = FLEX_SPI_CSR_BITS_8_BIT | FLEX_SPI_CSR_NCPHA(0) | FLEX_SPI_CSR_CPOL(0);

    // Enables the Transmit and Receive FIFOs
    FLEXCOM3_REGS->FLEX_SPI_CR = FLEX_SPI_CR_FIFOEN(1);

    // Transmit FIFO Threshold = 4 bytes
    //FLEXCOM3_REGS->FLEX_SPI_FMR = FLEX_SPI_FMR_TXFTHRES(4);

    // Enable Interrupts
    NVIC_SetPriority(FLEXCOM3_IRQn, 5);
    NVIC_EnableIRQ(FLEXCOM3_IRQn);

    // Enable SPI
    //FLEXCOM3_REGS->FLEX_SPI_IER = FLEX_SPI_IER_TDRE_Msk;
    FLEXCOM3_REGS->FLEX_SPI_CR = FLEX_SPI_CR_SPIEN(1);
}

void spi3_transmit (char* buf, int n) {
    tx_ptr = buf;
    tx_cntr = n;

    FLEXCOM3_REGS->FLEX_SPI_IER = FLEX_SPI_IER_TDRE_Msk;
    //FLEXCOM3_REGS->FLEX_SPI_CR = FLEX_SPI_CR_SPIEN(1);
}

int spi3_tx_remaining_bytes(void) {
    return tx_cntr;
}

void FLEXCOM3_Handler(void)
{
    while (FLEXCOM3_REGS->FLEX_SPI_SR & FLEX_SPI_SR_TDRE_Msk) {
        // Write BYTE (!) to transmit
        if (tx_cntr > 0) {
            *((char *)&(FLEXCOM3_REGS->FLEX_SPI_TDR)) = *tx_ptr++;
            tx_cntr--;
        }
        else {
            FLEXCOM3_REGS->FLEX_SPI_IDR = FLEX_SPI_IDR_TDRE_Msk;
            //FLEXCOM3_REGS->FLEX_SPI_CR = FLEX_SPI_CR_SPIDIS(1);
            break;
        }
    }
}