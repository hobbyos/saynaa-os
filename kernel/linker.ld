ENTRY(_start)

SECTIONS {
    . = 1M; /* Load address: 0x00100000 */

    .text : {
        *(.multiboot)   /* Multiboot header */
        *(.text*)       /* Bootloader and kernel text */
    }

    .rodata : {
        *(.rodata*)     /* Read-only data */
    }

    .data : {
        *(.data*)       /* Initialized data */
    }

    .bss : {
        *(.bss*)
        *(COMMON)
    }
}
