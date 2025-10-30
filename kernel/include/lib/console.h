#pragma once

#include "kernel/lib/vga.h"
#include "libc/string.h"

#define MAXIMUM_PAGES 16

#define SCROLL_UP 1
#define SCROLL_DOWN 2

void console_clear(VGA_COLOR_TYPE fore_color, VGA_COLOR_TYPE back_color);

// initialize console
void console_init(VGA_COLOR_TYPE fore_color, VGA_COLOR_TYPE back_color);
void console_scroll(int line_count);
void console_putchar(char ch);
// revert back the printed character and add 0 to it
void console_ungetchar();
// revert back the printed character until n characters
void console_ungetchar_bound(uint8_t n);

void console_gotoxy(uint16_t x, uint16_t y);

void console_putstr(const char* str);
