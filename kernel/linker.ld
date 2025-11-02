ENTRY(_start)

SECTIONS {

    . = 0xC0100000;

	__kernel_start_virt__ = .;
	__kernel_start_phys__ = . - 0xC0000000;

    .text : AT(ADDR(.text) - 0xC0000000) {
		*(.multiboot)
		*(.text)
	}

    .rodata ALIGN(4K) : AT(ADDR(.rodata) - 0xC0000000) {
		*(.rodata)
	}

    .data ALIGN(4K) : AT(ADDR(.data) - 0xC0000000) {
		*(.data)
	}

    .bss : AT(ADDR(.bss) - 0xC0000000) {
		*(.bss)
	}

    __kernel_end_virt__ = .;
	__kernel_end_phys__ = . - 0xC0000000;
	__kernel_size__     = __kernel_end_virt__ - __kernel_start_virt__;
}
