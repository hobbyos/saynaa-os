#pragma once

#include "libc/stdint.h"

void memset(void* ptr, int value, uint32_t size);
void memcpy(void* dst, const void* src, uint32_t n);
int strlen(const char* s);
int strncmp(const char* s1, const char* s2, size_t n);