#pragma once

#include "libc/stdint.h"

// Returns the absolute value of a signed integer.
int abs(int num);

// Performs unsigned 64-bit integer division (n / d) and returns the quotient.
uint64_t __udivdi3(uint64_t n, uint64_t d);

// Performs unsigned 64-bit integer modulus (n % d) and returns the remainder.
uint64_t __umoddi3(uint64_t n, uint64_t d);

// Performs signed 64-bit integer division (a / b) using unsigned division helper.
int64_t __divdi3(int64_t a, int64_t b);

// Performs signed 64-bit integer modulus (a % b) using unsigned modulus helper.
int64_t __moddi3(int64_t a, int64_t b);