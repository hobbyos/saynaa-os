#include "libc/string.h"

char* strrchr(const char* s, int c) {
    uint32_t n = strlen(s);
    if (!n) {
        return NULL;
    }
    for (int i = n - 1; i >= 0; i--) {
        if (s[i] == c) {
            return (char*) &s[i];
        }
    }
    return NULL;
}