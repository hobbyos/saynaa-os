#include "kernel/kernel.h"

#include "kernel/boot/multiboot2.h"
#include "kernel/cpu/fpu.h"
#include "kernel/cpu/gdt.h"
#include "kernel/cpu/idt.h"
#include "kernel/cpu/serial.h"
#include "kernel/lib/fb.h"
#include "kernel/lib/kprintf.h"

void kernel_main(mb2_t* boot, uint32_t magic) {
    init_serial();
    init_fpu();
    init_fb(boot);
    set_text_color(vga_to_color(0), vga_to_color(15));

    init_gdt();
    init_idt();

    if (magic != MB2_MAGIC) {
        kprintf("warning: invalid magic number from GRUB (%p)!", magic);
    }

    kprintf("Saynaa OS, from scratch\n\n");
    kprintf("PI value %.10f\n", 3.1415926535);

    infinite_loop();
}
