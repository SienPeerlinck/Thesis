#include "sam.h"

// GCLK_DIV = 100-1 => Timer GClk = 10 MHz / 100 = 100 kHz
#define GCLK_DIV 100
#define T0CH0_HALF_PERIOD 50

void init_timer0(void);
unsigned int get_timer0_cnt(void);

// Timer setup for FreeRTOS Run Time Statistics
// Timer0Ch0 & Timer0Ch1 chained
// TIOA0 = 10 MHz / 100 / (50 * 2) = 1kHz

void init_timer0(void)
{
    // Enable Peripheral Clock for TC0 (id=25), GCLK source = MAIN_CLK,
    PMC_REGS->PMC_PCR = PMC_PCR_EN(1) | PMC_PCR_GCLKCSS_MAIN_CLK | PMC_PCR_GCLKDIV(GCLK_DIV - 1) | PMC_PCR_CMD(1) | PMC_PCR_PID(25);

    // Enable GCLK for TC0 (id=25)
    PMC_REGS->PMC_PCR = PMC_PCR_EN(1) | PMC_PCR_GCLKEN(1) | PMC_PCR_GCLKCSS_MAIN_CLK | PMC_PCR_GCLKDIV(GCLK_DIV - 1) | PMC_PCR_CMD(1) | PMC_PCR_PID(25);

    // Enable Peripheral Clock for TC1 (id=26)
    PMC_REGS->PMC_PCR = PMC_PCR_EN(1) | PMC_PCR_CMD(1) | PMC_PCR_PID(26);

    // TC0 Chan0 config: Waveform mode + count up to RC (50) + Toggle TIOA0 output @ RC
    // TIOA0 =  100 kHz / (50 * 2) = 1 kHz
    TC0_REGS->TC_CHANNEL[0].TC_CMR = TC_CMR_WAVE(1) | TC_CMR_WAVEFORM_WAVSEL_UP_RC | TC_CMR_WAVEFORM_ACPC_TOGGLE;

    // TC0 Half Period: 50
    TC0_REGS->TC_CHANNEL[0].TC_RC = T0CH0_HALF_PERIOD;

    // Chain: TC0_Ch0 -> TC0_Ch1
    TC0_REGS->TC_BMR |= TC_BMR_TC1XC1S_TIOA0;

    // Clock source for Timer0 Ch1 is XC1 (TIOA0 output from Timer0 Ch0)
    TC0_REGS->TC_CHANNEL[1].TC_CMR = TC_CMR_WAVE(1) | TC_CMR_TCCLKS_XC1;

    // Clock Enable
    TC0_REGS->TC_CHANNEL[0].TC_CCR = TC_CCR_CLKEN(1);
    TC0_REGS->TC_CHANNEL[1].TC_CCR = TC_CCR_CLKEN(1);

    //  Start Timers
    TC0_REGS->TC_BCR = TC_BCR_SYNC(1);
}

inline unsigned int get_timer0_cnt(void)
{
    return TC0_REGS->TC_CHANNEL[1].TC_CV;
}
