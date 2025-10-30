#include "kernel/cpu/tss.h"

#include "kernel/cpu/gdt.h"
#include "libc/string.h"

extern void load_tss();

tss_entry_t tss;

/**
 * Writes a Task State Segment (TSS) entry into the Global Descriptor Table (GDT).
 * @param num  GDT index where the TSS will be stored.
 * @param ss0  Kernel mode stack segment.
 * @param esp0 Kernel mode stack pointer.
 */
void write_tss(int num, uint16_t ss0, uint32_t esp0) {
    uint32_t base = (uint32_t) &tss;
    uint32_t limit = sizeof(tss);

    // Add the TSS descriptor to the GDT.
    gdt_set_entry(num, base, limit, 0xE9, 0x00);

    // Clear the TSS.
    memset(&tss, 0, sizeof(tss_entry_t));

    // Set the kernel stack segment and stack pointer.
    tss.ss0 = ss0;
    tss.esp0 = esp0;

    // Set segment selectors for code and data segments.
    tss.cs = 0x0B;                                     // Code segment selector (Ring 3).
    tss.ss = tss.ds = tss.es = tss.fs = tss.gs = 0x13; // Data segment selectors.

    // Set the I/O map base address to the end of the TSS structure.
    tss.iomap_base = sizeof(tss_entry_t);
}

// Update the kernel stack pointer in the TSS.
void set_kernel_stack(uint32_t stack) {
    tss.esp0 = stack;
}
