#include "kernel/lib/kprintf.h"

#include "kernel/cpu/serial.h"
#include "kernel/lib/fb.h"
#include "kernel/lib/font.h"
#include "libc/string.h"

int pos_x = 0, pos_y = 0;
int pos_x2 = 0;

uint32_t fore_color = 0xffffffff;
uint32_t back_color = 0xff000000;

#define print_char(c) vbe_print_char(c)

void set_pos_text(int x, int y) {
    pos_x = x;
    pos_x2 = pos_x;
    pos_y = y;
}

void set_text_color(uint32_t fg, uint32_t bg) {
    fore_color = fg;
    back_color = bg;
}

void print_ch(char c) {
    int lx;
    int ly;
    uint8_t* bitmap = (uint8_t*) font8x8_basic[c % 128];
    for (lx = 0; lx < GLYPH_WIDTH; lx++) {
        for (ly = 0; ly < GLYPH_HEIGHT; ly++) {
            uint8_t row = bitmap[ly];
            if ((row >> lx) & 1)
                draw_pixel(pos_x + lx, pos_y + ly, fore_color);
            else
                draw_pixel(pos_x + lx, pos_y + ly, back_color);
        }
    }
}

void vbe_print_char(char c) {
    write_serial(c);
    if (c == '\n') {
        pos_y += GLYPH_HEIGHT;
        pos_x = pos_x2;
    } else if (c == '\b') {
        pos_x -= GLYPH_WIDTH;
    } else if (c == '\r') {
        pos_x = pos_x2;
    } else {
        print_ch(c);
        pos_x += GLYPH_WIDTH;
    }
}

void put_string(char* s) {
    uint32_t l = strlen(s);
    for (uint32_t i = 0; i < l; i++) {
        char c = s[i];
        vbe_print_char(c);
    }
}

int kprintf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    char text[4096] = {0};
    vsprintf(text, fmt, args);
    put_string(text);

    va_end(args);
    return 0;
}