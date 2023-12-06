STACK_SIZE = 0x400;

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
SEARCH_DIR(.)

/* Memory Spaces Definitions */
MEMORY
{
ITCM (rwx)     : ORIGIN = 0x00000000, LENGTH = 0x00020000 /* ITCM, 128K */
DTCM (rwx)     : ORIGIN = 0x20000000, LENGTH = 0x00040000 /* DTCM, 256K */
Flash (rx)     : ORIGIN = 0x10000000, LENGTH = 0x00020000 /* Flash, 128K */
FlexRAM (rwx)  : ORIGIN = 0x21000000, LENGTH = 0x000C0000 /* FlexRAM, 768K */
BootPROM (rwx) : ORIGIN = 0x60000000, LENGTH = 0x00080000 /* SST39VF040 512K x 8 Flash */
}

/* Section Definitions */
SECTIONS
{
    .text :
    {
        . = ALIGN(4);
        _vec_table = .;
        KEEP(*(.vectors .vectors.*))
        . = ALIGN(4);
        _e_vec_table = .;
        *(.boot .boot.*)
       . = ALIGN(4);
       _etext = .;
    } > Flash

    /* Data section to be copied from Flash -> DTCM */
    .relocate_d :
    {
        . = ALIGN(4);
        _srelocate_d = .;
        *(.data .data.*);
        . = ALIGN(4);
        _erelocate_d = .;
    } > DTCM AT> Flash

    .vec_table_i :
    {
        . = ALIGN(4);
        _vec_table_itcm = .;
        /* KEEP(*(.vectors_itcm .vectors_itcm.*))*/
        . = _vec_table_itcm + 0x200;
        . = ALIGN(4);
        _e_vec_table_itcm = .;
    } > ITCM


    /* Code section to be copied from Flash -> ITCM */
    .relocate_i :
    {
        . = ALIGN(4);
        _srelocate_i = .;
        *(.text .text.*)    
        *(.init)
        *(.fini)
        *(.rodata*)
        . = ALIGN(4);
        _erelocate_i = .;
    } > ITCM AT> Flash
    
    .end_of_flash :
    {
       . = ALIGN(4);
       _eflash = .;
    } > Flash

    /* .bss section which is used for uninitialized data */
    .bss (NOLOAD) :
    {
        . = ALIGN(4);
        _szero = .;
        *(.bss .bss.*)
        *(COMMON)
        . = ALIGN(4);
        _ezero = .;
    } > DTCM

    .heap (NOLOAD) :
    {
        . = ALIGN(4);
        _sheap = .;
        . = ORIGIN(DTCM) + LENGTH(DTCM) - STACK_SIZE;
        _eheap = .;
    } > DTCM

    .flexram (NOLOAD) :
    {
        . = ALIGN(4);
        *(.flexram_nocache .flexram_nocache.*)
    } > FlexRAM

    _estack = ORIGIN(DTCM) + LENGTH(DTCM);
}
