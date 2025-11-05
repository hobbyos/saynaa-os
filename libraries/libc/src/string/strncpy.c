#include "libc/string.h"

char* strncpy(char* dest, const char* src, size_t n) {
    uint32_t len = strnlen(src, n);

    if (len != n) {
        memset(dest + len, '\0', n - len);
    }

    memcpy(dest, src, len);

    return dest;
}