ENTRY(_start)

PHDRS {
    text PT_LOAD FLAGS(5);  /* Read + Execute */
    data PT_LOAD FLAGS(6);  /* Read + Write */
    bss  PT_LOAD FLAGS(6);  /* Read + Write */
}

SECTIONS {
    . = 1M; /* Load address: 0x00100000 */

    .text : {
        *(.multiboot)   /* Multiboot header */
        *(.text*)       /* Bootloader and kernel text */
    } :text             /* assign to PHDR "text" */

    .rodata : {
        *(.rodata*)     /* Read-only data */
    } :text             /* assign to PHDR "text" */

    .data : {
        *(.data*)       /* Initialized data */
    } :data             /* assign to PHDR "data" */

    .bss : {
        *(.bss*)
        *(COMMON)
    } :bss              /* assign to PHDR "bss" */
}
