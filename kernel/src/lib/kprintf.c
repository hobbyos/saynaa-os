#include "kernel/lib/kprintf.h"

#include "kernel/lib/console.h"

int kprintf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    char text[4096] = {0};
    vsprintf(text, fmt, args);
    console_putstr(text);

    va_end(args);
    return 0;
}