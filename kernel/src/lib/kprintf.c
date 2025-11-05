#include "kernel/lib/kprintf.h"

#include "kernel/cpu/serial.h"
#include "kernel/lib/fb.h"
#include "kernel/lib/font.h"
#include "libc/string.h"

int font_scale = 1;
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

void set_font_scale(int scale) {
    if (scale < 1)
        scale = 1;
    font_scale = scale;
}

void print_ch(char c) {
    int lx, ly, sx, sy;
    uint8_t* bitmap = (uint8_t*) font8x8_basic[c % 128];
    for (ly = 0; ly < GLYPH_HEIGHT; ly++) {
        uint8_t row = bitmap[ly];
        for (lx = 0; lx < GLYPH_WIDTH; lx++) {
            uint32_t color = ((row >> lx) & 1) ? fore_color : back_color;

            // scale each pixel into a block of pixels
            for (sy = 0; sy < font_scale; sy++) {
                for (sx = 0; sx < font_scale; sx++) {
                    draw_pixel(pos_x + lx * font_scale + sx, pos_y + ly * font_scale + sy, color);
                }
            }
        }
    }
}

void vbe_print_char(char c) {
    write_serial(c);
    if (c == '\n') {
        pos_y += GLYPH_HEIGHT * font_scale;
        pos_x = pos_x2;
    } else if (c == '\b') {
        pos_x -= GLYPH_WIDTH * font_scale;
    } else if (c == '\r') {
        pos_x = pos_x2;
    } else {
        print_ch(c);
        pos_x += GLYPH_WIDTH * font_scale;
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