#include "kernel/kernel.h"

#include "kernel/cpu/gdt.h"
#include "kernel/cpu/idt.h"
#include "kernel/lib/console.h"
#include "kernel/lib/kprintf.h"
#include "kernel/lib/vga.h"

void kernel_main() {
    init_gdt();
    init_idt();

    console_init(COLOR_WHITE, COLOR_BLACK);

    // raise division by zero, eax=0x7b, ecx=0
    // asm volatile("\txorl %edx, %edx");
    // asm volatile("\tmovl $0x7b, %eax");
    // asm volatile("\tmovl $0, %ecx");
    // asm volatile("\tidivl %ecx");

    kprintf("Saynaa OS, from scratch\n");

    infinite_loop();
}
