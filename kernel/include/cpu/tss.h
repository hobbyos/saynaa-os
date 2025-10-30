#pragma once

#include "libc/stdint.h"

/**
 * Structure representing the Task State Segment (TSS).
 * Holds information for multitasking and privilege-level transitions.
 */
typedef struct {
    uint32_t prev_tss;               // Previous TSS (not used in modern OS).
    uint32_t esp0;                   // Stack pointer for kernel mode.
    uint32_t ss0;                    // Stack segment for kernel mode.
    uint32_t esp1;                   // Stack pointer for Ring 1 (unused).
    uint32_t ss1;                    // Stack segment for Ring 1 (unused).
    uint32_t esp2;                   // Stack pointer for Ring 2 (unused).
    uint32_t ss2;                    // Stack segment for Ring 2 (unused).
    uint32_t cr3;                    // Page directory base register.
    uint32_t eip;                    // Instruction pointer.
    uint32_t eflags;                 // CPU flags register.
    uint32_t eax, ecx, edx, ebx;     // General-purpose registers.
    uint32_t esp, ebp;               // Stack pointer and base pointer.
    uint32_t esi, edi;               // Additional general-purpose registers.
    uint32_t es, cs, ss, ds, fs, gs; // Segment registers.
    uint32_t ldt;                    // Local Descriptor Table selector.
    uint16_t trap;                   // Trap flag.
    uint16_t iomap_base;             // I/O map base address.
} __attribute__((packed)) tss_entry_t;

extern void load_tss();                                      // Load the TSS.
extern void write_tss(int num, uint16_t ss0, uint32_t esp0); // Write TSS entry.
extern void set_kernel_stack(uint32_t stack);                // Set the kernel stack pointer.
