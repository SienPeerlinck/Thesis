#ifndef INC_EXT_FLASH_H
#define INC_EXT_FLASH_H

// from Arm® Cortex®-M7 Devices Generic User Guide
// Table 2.11
//
// | Address range | Memory region          | Memory type      | Description
// |---------------|------------------------|------------------|-----------------------------------------------
// | 0x00000000-   |                        |                  |
// | 0x1FFFFFFF    | Code                   | Normal           | Executable region for program code. You can also put data here.
// | 0x20000000-   |                        |                  |
// | 0x3FFFFFFF    | SRAM                   | Normal           |  Executable region for data. You can also put code here.
// | 0x40000000-   |                        |                  |
// | 0x5FFFFFFF    | Peripheral             | Device           |  Peripheral address space.
// | 0x60000000-   |                        |                  |
// | 0x9FFFFFFF    | External RAM           | Normal           | Executable region for data. You can also put code here.
// | 0xA0000000-   |                        |                  |
// | 0xDFFFFFFF    | External device        | Device           | External Device memory.
// | 0xE0000000-   |                        |                  |
// | 0xE00FFFFF    | Private Peripheral Bus | Strongly Ordered | This region includes the NVIC, System timer, and system control block.
// | 0xE0100000-   |                        |                  |
// | 0xFFFFFFFF    | Vendor-specific device | Device           | Accesses to this region are to vendor-specific peripherals.
// |_______________|________________________|__________________|__________________________________________

#define EXT_FLASH_ADDR 0xA0000000

void init_ext_flash(void);
void write_one_byte(volatile char *addr, char data);
void erase_sector(volatile char *addr);
unsigned int copy_embedded_flash_to_ext_flash(void);

#endif