#include "libc/math.h"

int min(int a, int b) {
    return a < b ? a : b;
}

int max(int a, int b) {
    return a > b ? a : b;
}

uint32_t align_to(uint32_t n, uint32_t align) {
    if (n % align == 0) {
        return n;
    }
    return n + (align - n % align);
}

uint32_t divide_up(uint32_t n, uint32_t d) {
    if (n % d == 0) {
        return n / d;
    }
    return 1 + n / d;
}
