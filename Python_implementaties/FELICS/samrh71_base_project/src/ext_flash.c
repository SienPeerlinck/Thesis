#include "sam.h"
#include "ext_flash.h"

#include "opt/printf-stdarg.h"

// SST39VF040-70
// Read Cycle 70 ns

extern unsigned int _vec_table;
extern unsigned int _eflash;
extern void Dummy_Handler(void);
extern char boot_status; // 0 => Boot from Embedded Flash, 1 => Boot from External Flash.


#define NCS0_ADDBASE ((EXT_FLASH_ADDR - 0x60000000) >> 14)

void init_ext_flash(void)
{
    HEMC_REGS->HEMC_CR_NCS0 = HEMC_CR_NCS0_ADDBASE(NCS0_ADDBASE) | HEMC_CR_NCS0_BANKSIZE_512KB;
    // Memory timings with MCK = 50MHZ ==> unit = 20ns
    HSMC_REGS->HSMC_CS[0].HSMC_SETUP = HSMC_SETUP_NWE_SETUP(2) | HSMC_SETUP_NCS_WR_SETUP(1) |
                                       HSMC_SETUP_NRD_SETUP(2) | HSMC_SETUP_NCS_RD_SETUP(1);
    HSMC_REGS->HSMC_CS[0].HSMC_PULSE = HSMC_PULSE_NWE_PULSE(4) | HSMC_PULSE_NCS_WR_PULSE(6) |
                                       HSMC_PULSE_NRD_PULSE(4) | HSMC_PULSE_NCS_RD_PULSE(6);
    HSMC_REGS->HSMC_CS[0].HSMC_CYCLE = HSMC_CYCLE_NWE_CYCLE(8) | HSMC_CYCLE_NRD_CYCLE(8);

    // 8bit data bus, write operation is controlled by the NWE signal.
    HSMC_REGS->HSMC_CS[0].HSMC_MODE = HSMC_MODE_DBW_8_BIT | HSMC_MODE_WRITE_MODE(1);
}

void write_one_byte(volatile char *addr, char data)
{
    char c;
    char dummy[4];
    // volatile char *addr = (char *)(EXT_FLASH_ADDR | (addr_offset & 0x7ffff));
    *(volatile char *)(EXT_FLASH_ADDR | 0x5555) = 0xaa;
    *(volatile char *)(EXT_FLASH_ADDR | 0x2aaa) = 0x55;
    *(volatile char *)(EXT_FLASH_ADDR | 0x5555) = 0xa0;
    *addr = data;

    
    while (1)
    {
        c = *addr;
        if (c == *addr)
            break;
    }
}

void erase_sector(volatile char *addr)
{
    char c;
    *(volatile char *)(EXT_FLASH_ADDR | 0x5555) = 0xaa;
    *(volatile char *)(EXT_FLASH_ADDR | 0x2aaa) = 0x55;
    *(volatile char *)(EXT_FLASH_ADDR | 0x5555) = 0x80;
    *(volatile char *)(EXT_FLASH_ADDR | 0x5555) = 0xaa;
    *(volatile char *)(EXT_FLASH_ADDR | 0x2aaa) = 0x55;
    *addr = 0x30;

    while (1)
    {
        c = *addr;
        if (c == *addr)
            break;
    }
}

unsigned int copy_embedded_flash_to_ext_flash(void)
{
    unsigned int *src_w_addr;
    if ((boot_status & 1) != 0) {
        printf("\nWe didn't boot from Embedded Flash ==> No Copy\n");
        return;
    }

    char *src = &_vec_table;
    char *dest = EXT_FLASH_ADDR;
    
    // Start power-on sequence of Embedded Flash
    HEFC_REGS->HEFC_FPMR |= HEFC_FPMR_PWS_EN_Msk;
    // Wait for the power-on sequence to complete
    while (! (HEFC_REGS->HEFC_FPMR & HEFC_FPMR_PWS_STAT_Msk));


    printf("\nCopy %08x - %08x -> %08x\n", src, &_eflash, dest);
    while (src < &_eflash) {
        if (((unsigned int) dest & 0xfff) == 0) {
            printf("Erase sector %08x\n", dest);
            erase_sector(dest);
        }

        if (((unsigned int)src & 3) == 3) {
            src_w_addr = (unsigned int*)((unsigned int)src & 0xfffffffc);
            if (*src_w_addr == &Reset_Handler) {
                write_one_byte(dest, 0x60);
                printf("Reset Handler @ %08x\n", src);
            }
            else if (*src_w_addr == &Dummy_Handler) {
                write_one_byte(dest, 0x60);
                printf("Dummy Handler @ %08x\n", src);
            }
            else
                write_one_byte(dest, *src);
        } else
                write_one_byte(dest, *src);

        dest++;
        src++;
    }
    printf("Copy done %08x %08x\n", src, dest);

    // Start power-off sequence of Embedded Flash
    HEFC_REGS->HEFC_FPMR &= ~HEFC_FPMR_PWS_EN_Msk;
}