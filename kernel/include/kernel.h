#pragma once

#include "libc/stdint.h"

#define enable_interrupts() asm volatile("sti")
#define disable_interrupts() asm volatile("cli")
#define halt() asm volatile("hlt")
#define unused(p) (void) p

#define infinite_loop() \
    while (1) { \
        halt(); \
    }
