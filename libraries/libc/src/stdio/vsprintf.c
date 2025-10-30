#include "libc/stdio.h"

int vsprintf(char* restrict str, const char* restrict format, va_list list) {
    return vsnprintf(str, INT32_MAX, format, list);
}
