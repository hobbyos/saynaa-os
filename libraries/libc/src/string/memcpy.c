#include "libc/string.h"

void memcpy(void* dst, const void* src, uint32_t n) {
    char* p = dst;
    const char* q = src;
    while (n--)
        *p++ = *q++;
}