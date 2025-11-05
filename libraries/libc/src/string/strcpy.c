#include "libc/string.h"

int strcpy(char* dest, const char* src) {
    int i = 0;
    while ((*dest++ = *src++) != 0)
        i++;
    return i;
}