#include "libc/stdlib.h"

uint64_t __udivdi3(uint64_t n, uint64_t d) {
    uint64_t q = 0, r = 0;
    for (int i = 63; i >= 0; i--) {
        r = (r << 1) | ((n >> i) & 1);
        if (r >= d) {
            r -= d;
            q |= (1ULL << i);
        }
    }
    return q;
}

uint64_t __umoddi3(uint64_t n, uint64_t d) {
    uint64_t r = 0;
    for (int i = 63; i >= 0; i--) {
        r = (r << 1) | ((n >> i) & 1);
        if (r >= d)
            r -= d;
    }
    return r;
}

int64_t __divdi3(int64_t a, int64_t b) {
    int sign = (a < 0) ^ (b < 0);
    uint64_t ua = (a < 0) ? -a : a;
    uint64_t ub = (b < 0) ? -b : b;
    uint64_t q = __udivdi3(ua, ub);
    return sign ? -q : q;
}

int64_t __moddi3(int64_t a, int64_t b) {
    int sign = (a < 0);
    uint64_t ua = (a < 0) ? -a : a;
    uint64_t ub = (b < 0) ? -b : b;
    uint64_t r = __umoddi3(ua, ub);
    return sign ? -r : r;
}
