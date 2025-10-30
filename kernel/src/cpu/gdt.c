#include "kernel/cpu/gdt.h"

#include "kernel/cpu/tss.h"

// Global GDT array with the specified number of descriptors
GDT g_gdt[NO_GDT_DESCRIPTORS];

// GDT pointer that holds the limit and base address of the GDT
GDT_PTR g_gdt_ptr;

void gdt_set_entry(int index, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    GDT* this = &g_gdt[index];

    this->segment_limit = limit & 0xFFFF;                       // Lower 16 bits of limit
    this->base_low = base & 0xFFFF;                             // Lower 16 bits of base
    this->base_middle = (base >> 16) & 0xFF;                    // Next 8 bits of base
    this->access = access;                                      // Access flags
    this->granularity = ((limit >> 16) & 0x0F) | (gran & 0xF0); // Upper 4 bits of limit and granularity flags
    this->base_high = (base >> 24) & 0xFF;                      // Highest 8 bits of base
}

void init_gdt() {
    // Set GDT limit and base address
    g_gdt_ptr.limit = sizeof(g_gdt) - 1;
    g_gdt_ptr.base_address = (uint32_t) g_gdt;

    // Null segment
    gdt_set_entry(0, 0, 0, 0, 0);

    // Kernel code segment
    gdt_set_entry(1, 0, 0xFFFFFFFF, GDT_ACCESS_KERNEL_CODE, GDT_GRAND_FLAGS);
    // Kernel data segment
    gdt_set_entry(2, 0, 0xFFFFFFFF, GDT_ACCESS_KERNEL_DATA, GDT_GRAND_FLAGS);
    // User code segment
    gdt_set_entry(3, 0, 0xFFFFFFFF, GDT_ACCESS_USER_CODE, GDT_GRAND_FLAGS);
    // User data segment
    gdt_set_entry(4, 0, 0xFFFFFFFF, GDT_ACCESS_USER_DATA, GDT_GRAND_FLAGS);

    // Initialize Task State Segment (TSS)
    write_tss(5, 0x10, 0x0);

    // Load the GDT into the CPU
    load_gdt((uint32_t) &g_gdt_ptr);
    // Load the TSS
    load_tss();
}
