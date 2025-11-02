#pragma once

#include "libc/stdint.h"

void* kamalloc(uint32_t size, uint32_t align);
void* aligned_alloc(size_t align, size_t size);
void* malloc(size_t size);
void* realloc(void* ptr, size_t size);
void free(void* pointer);
uint32_t memory_usage();
