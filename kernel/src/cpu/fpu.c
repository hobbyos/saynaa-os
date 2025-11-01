#include "kernel/cpu/fpu.h"

#include "kernel/cpu/isr.h"
#include "kernel/kernel.h"
#include "kernel/lib/kprintf.h"

/* Buffer for FPU context, must be 16-bytes aligned */
static uint8_t kernel_fpu[512] __attribute__((aligned(16)));

void fpu_exception_handler(REGISTERS* regs);

// Function to initialize the FPU
void init_fpu() {
    uint32_t cr0, cr4;

    // Enable the FPU
    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 &= ~(1 << 2); // Clear EM (bit 2)
    cr0 |= (1 << 1);  // Set MP (bit 1)
    asm volatile("mov %0, %%cr0" ::"r"(cr0));

    // Enable SSE and FXSAVE/FXRSTOR
    asm volatile("mov %%cr4, %0" : "=r"(cr4));
    cr4 |= (3 << 9); // Set OSFXSR (bit 9) and OSXMMEXCPT (bit 10)
    asm volatile("mov %0, %%cr4" ::"r"(cr4));

    // Initialize the FPU
    asm volatile("fninit");

    // Register FPU exception handler
    isr_register_handler(19, fpu_exception_handler);
}

// Save the FPU state and initialize it for kernel use
void fpu_kernel_enter() {
    asm volatile("fxsave (%0)\n"
                 "fninit\n" ::"r"(kernel_fpu));
}

// Restore the FPU state when exiting the kernel
void fpu_kernel_exit() {
    asm volatile("fxrstor (%0)" ::"r"(kernel_fpu));
}

// Handler for FPU exceptions
void fpu_exception_handler(REGISTERS* regs) {
    unused(regs);
    kprintf("An FPU exception occurred");
}