#pragma once

#include "libc/stdint.h"

int min(int a, int b);

int max(int a, int b);

/* Round n up to the next multiple of align (or return n if already aligned). */
uint32_t align_to(uint32_t n, uint32_t align);

/* Divide n by d, rounding up. */
uint32_t divide_up(uint32_t n, uint32_t d);