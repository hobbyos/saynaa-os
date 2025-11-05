#include "libc/string.h"

size_t strnlen(const char* string, size_t max_len) {
    uint32_t result = 0;

    while (result < max_len && string[result]) {
        result++;
    }

    return result;
}