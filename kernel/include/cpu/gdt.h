#pragma once

#include "libc/stdint.h"

#define NO_GDT_DESCRIPTORS 6

// GDT access flags.
#define GDT_READWRITE (1 << 1) // Read/write access.
#define GDT_EXPANSION (1 << 2) // Expansion direction for data segments.
#define GDT_EXEC_CODE (1 << 3) // Executable code segment.
#define GDT_CODE_DATA (1 << 4) // Code/data segment.
#define GDT_GPL(n) (n << 5)    // Privilege level (0-3).
#define GDT_MEMORY (1 << 7)    // Segment present in memory.

// Granularity flags.
#define GDT_GRAND_LIMIT_HI 0x0f // High 4 bits of the segment limit.
#define GDT_GRAND_OS 0x10       // Reserved for OS use.
#define GDT_GRAND_32BIT 0x40    // 32-bit segment.
#define GDT_GRAND_4K 0x80       // 4KB granularity.

// Combine multiple granularity flags.
#define GDT_GRAND_FLAGS (GDT_GRAND_4K | GDT_GRAND_32BIT | GDT_GRAND_LIMIT_HI)

// Access permissions for kernel and user segments.
#define GDT_ACCESS_KERNEL_CODE (GDT_READWRITE | GDT_EXEC_CODE | GDT_CODE_DATA | GDT_MEMORY)
#define GDT_ACCESS_KERNEL_DATA (GDT_READWRITE | GDT_CODE_DATA | GDT_MEMORY)
#define GDT_ACCESS_USER_CODE \
    (GDT_READWRITE | GDT_EXEC_CODE | GDT_GPL(3) | GDT_CODE_DATA | GDT_MEMORY)
#define GDT_ACCESS_USER_DATA (GDT_READWRITE | GDT_CODE_DATA | GDT_GPL(3) | GDT_MEMORY)

// Structure representing a single GDT entry.
typedef struct {
    uint16_t segment_limit;    // Lower 16 bits of the segment limit.
    uint16_t base_low;         // Lower 16 bits of the base address.
    uint8_t base_middle;       // Middle 8 bits of the base address.
    uint8_t access;            // Access flags.
    uint8_t granularity;       // Granularity flags and upper segment limit bits.
    uint8_t base_high;         // Upper 8 bits of the base address.
} __attribute__((packed)) GDT; // Packed to avoid padding between fields.

// Structure representing the GDT pointer (used to load the GDT).
typedef struct {
    uint16_t limit;                // Total size of all GDT entries.
    uint32_t base_address;         // Address of the first GDT entry.
} __attribute__((packed)) GDT_PTR; // Packed to ensure no padding between fields.

// Assembly function to load the GDT using its pointer.
extern void load_gdt(uint32_t gdt_ptr);

/**
 * Sets the properties of a GDT entry.
 * @param index - Index of the GDT entry.
 * @param base - Base address of the segment.
 * @param limit - Size (limit) of the segment.
 * @param access - Access flags.
 * @param gran - Granularity flags.
 */
void gdt_set_entry(int index, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran);

/**
 * Initializes the GDT by setting all entries and loading it into the CPU.
 */
void init_gdt();
