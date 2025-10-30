#pragma once

#include "libc/stdint.h"

#define NO_IDT_DESCRIPTORS 256

#define IDT_BIT16 0x06   // 16-bit interrupt gate
#define IDT_BIT32 0x0E   // 32-bit interrupt gate
#define IDT_RING1 0x40   // Ring 1 privilege level
#define IDT_RING2 0x20   // Ring 2 privilege level
#define IDT_RING3 0x60   // Ring 3 privilege level
#define IDT_PRESENT 0x80 // Present flag

#define DISPATCHER_ISR 0x7F

#define IDT_FLAGS (IDT_PRESENT | IDT_BIT32)

typedef struct {
    uint16_t base_low;         // Lower 16 bits of the handler function address
    uint16_t segment_selector; // Code segment selector in GDT
    uint8_t zero;              // Unused, set to zero
    uint8_t type;              // Descriptor type and attributes
    uint16_t base_high;        // Upper 16 bits of the handler function address
} __attribute__((packed)) IDT;

typedef struct {
    uint16_t limit;        // Limit size of all IDT segments
    uint32_t base_address; // Base address of the first IDT segment
} __attribute__((packed)) IDT_PTR;

// Assembly function to load the IDT, defined in load_idt.asm
extern void load_idt(uint32_t idt_ptr);

/**
 * Set an entry in the IDT
 */
void idt_set_entry(int index, uint32_t base, uint16_t seg_sel, uint8_t flags);

void init_idt();
