#include "obt.h"
#include "opt/printf-stdarg.h"
#include "sam.h"
#include "utils.h"

static int last_delta_t;

void set_obt(Obt_t *obt)
{
    //PMC_REGS->PMC_PCR = PMC_PCR_EN(1) | PMC_PCR_CMD(1) | PMC_PCR_PID(2);
    // Wait for a second periodic event
    RTC_REGS->RTC_SCCR = RTC_SCCR_SECCLR(1);
    while ((RTC_REGS->RTC_SR & RTC_SR_SEC_Msk) == 0); // vTaskDelay(1);           
    // Enter Update mode
    RTC_REGS->RTC_CR = RTC_CR_UPDTIM(1) | RTC_CR_UPDCAL(1);
    // Wait for Update mode Acknowledgement
    while ((RTC_REGS->RTC_SR & RTC_SR_ACKUPD_Msk) == 0);
    RTC_REGS->RTC_SCCR = RTC_SCCR_ACKCLR(1);
    // Update date & time
    RTC_REGS->RTC_TIMR = obt->timr;
    RTC_REGS->RTC_CALR = obt->calr;
    // Quit Update mode
    RTC_REGS->RTC_CR = RTC_CR_UPDTIM(0) | RTC_CR_UPDCAL(0);
}

void get_obt(Obt_t *obt)
{
    unsigned int prev_timr;
    unsigned int prev_calr;
    unsigned int prev_msr;

    // From Datasheet 24.5.2 
    // To be certain that the value read in the RTC registers are valid and stable,
    // it is necessary to read these registers twice.
    // If the data is the same both times, then it is valid.
    while (1) {
        obt->timr = RTC_REGS->RTC_TIMR;
        obt->calr = RTC_REGS->RTC_CALR;
        obt->msr = RTC_REGS->RTC_MSR;
        if (obt->timr == prev_timr && obt->calr == prev_calr && obt->msr == prev_msr)
          break;
        prev_timr = obt->timr;
        prev_calr = obt->calr;
        prev_msr = obt->msr;
    }
}


void calib_obt(Obt_t *obt, Obt_t *ref_obt)
{
    int msr, ref_msr, delta_t;
    int corr; 
    
    msr = obt->msr + 1024 * from_bcd(obt->timr & 0x7f);
    ref_msr = ref_obt->msr + 1024 * from_bcd(ref_obt->timr & 0x7f);
    delta_t = ref_msr - msr;
    if (delta_t > 30 * 1024) delta_t -= 60 * 1024;
    else if (delta_t < -30 * 1024) delta_t += 60 * 1024;
    last_delta_t = delta_t;
    
    if (delta_t > 0) {
        if (delta_t < 10) {
            corr = (9 - delta_t) * 15;
            RTC_REGS->RTC_MR = RTC_MR_CORRECTION(corr);
        }
        else
            RTC_REGS->RTC_MR = RTC_MR_HIGHPPM(1) | RTC_MR_CORRECTION(1);
    }
    else {
        if (delta_t > -10) {
            corr = (9 + delta_t) * 15;
            RTC_REGS->RTC_MR = RTC_MR_CORRECTION(corr) | RTC_MR_NEGPPM(1);
        }
        else
            RTC_REGS->RTC_MR = RTC_MR_HIGHPPM(1) | RTC_MR_CORRECTION(1) | RTC_MR_NEGPPM(1);
    }
    
}

void print_obt(Obt_t *obt) {
    printf("%02x%02x-%02x-%02xT%02x:%02x:%02x.%06d   (delta=%d)",
        obt->calr & 0x7f, (obt->calr >> 8) & 0xff, (obt->calr >> 16) & 0x1f, (obt->calr >> 24) & 0x3f,
        (obt->timr >> 16) & 0x3f, (obt->timr >> 8) & 0x7f, obt->timr & 0x7f,
        obt->msr * 1000000 / 1024,
        last_delta_t);
}
