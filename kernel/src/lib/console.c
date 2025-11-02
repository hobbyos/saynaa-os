#include "kernel/lib/console.h"

#include "kernel/cpu/serial.h"
#include "kernel/lib/kprintf.h"

static uint16_t* g_vga_buffer;
// index for video buffer array
static uint32_t g_vga_index;
// cursor positions
static uint8_t cursor_pos_x = 0, cursor_pos_y = 0;
// fore & back color values
uint8_t g_fore_color = COLOR_WHITE, g_back_color = COLOR_BLACK;
static uint16_t g_temp_pages[MAXIMUM_PAGES][VGA_TOTAL_ITEMS];
uint32_t g_current_temp_page = 0;

// clear video buffer array
void console_clear(VGA_COLOR_TYPE fore_color, VGA_COLOR_TYPE back_color) {
    uint32_t i;

    for (i = 0; i < VGA_TOTAL_ITEMS; i++) {
        g_vga_buffer[i] = vga_item_entry(NULL, fore_color, back_color);
    }
    g_vga_index = 0;
    cursor_pos_x = 0;
    cursor_pos_y = 0;
    vga_set_cursor_pos(cursor_pos_x, cursor_pos_y);
}

// initialize console
void console_init(VGA_COLOR_TYPE fore_color, VGA_COLOR_TYPE back_color) {
    g_vga_buffer = (uint16_t*) VGA_ADDRESS;
    g_fore_color = fore_color;
    g_back_color = back_color;
    cursor_pos_x = 0;
    cursor_pos_y = 0;
    console_clear(fore_color, back_color);
}

void console_scroll(int type) {
    uint32_t i;
    if (type == SCROLL_UP) {
        // scroll up
        if (g_current_temp_page > 0)
            g_current_temp_page--;
        g_current_temp_page %= MAXIMUM_PAGES;
        for (i = 0; i < VGA_TOTAL_ITEMS; i++) {
            g_vga_buffer[i] = g_temp_pages[g_current_temp_page][i];
        }
    } else {
        // scroll down
        g_current_temp_page++;
        g_current_temp_page %= MAXIMUM_PAGES;
        for (i = 0; i < VGA_TOTAL_ITEMS; i++) {
            g_vga_buffer[i] = g_temp_pages[g_current_temp_page][i];
        }
    }
}

/*
increase vga_index by width of vga width
*/
static void console_newline() {
    uint32_t i;

    if (cursor_pos_y >= VGA_HEIGHT) {
        for (i = 0; i < VGA_TOTAL_ITEMS; i++)
            g_temp_pages[g_current_temp_page][i] = g_vga_buffer[i];
        g_current_temp_page++;
        cursor_pos_x = 0;
        cursor_pos_y = 0;
        console_clear(g_fore_color, g_back_color);
    } else {
        for (i = 0; i < VGA_TOTAL_ITEMS; i++)
            g_temp_pages[g_current_temp_page][i] = g_vga_buffer[i];

        g_vga_index += VGA_WIDTH - (g_vga_index % VGA_WIDTH);
        cursor_pos_x = 0;
        ++cursor_pos_y;
        vga_set_cursor_pos(cursor_pos_x, cursor_pos_y);
    }
}

// assign ascii character to video buffer
void console_putchar(char ch) {
    if (ch == ' ') {
        g_vga_buffer[g_vga_index++] = vga_item_entry(' ', g_fore_color, g_back_color);
        vga_set_cursor_pos(cursor_pos_x++, cursor_pos_y);
    }
    if (ch == '\t') {
        for (int i = 0; i < 4; i++) {
            g_vga_buffer[g_vga_index++] = vga_item_entry(' ', g_fore_color, g_back_color);
            vga_set_cursor_pos(cursor_pos_x++, cursor_pos_y);
        }
    } else if (ch == '\n') {
        console_newline();
    } else {
        if (ch > 0) {
            g_vga_buffer[g_vga_index++] = vga_item_entry(ch, g_fore_color, g_back_color);
            vga_set_cursor_pos(++cursor_pos_x, cursor_pos_y);
        }
    }
}

// revert back the printed character and add 0 to it
void console_ungetchar() {
    if (g_vga_index > 0) {
        g_vga_buffer[g_vga_index--] = vga_item_entry(0, g_fore_color, g_back_color);
        if (cursor_pos_x > 0) {
            vga_set_cursor_pos(cursor_pos_x--, cursor_pos_y);
        } else {
            cursor_pos_x = VGA_WIDTH;
            if (cursor_pos_y > 0)
                vga_set_cursor_pos(cursor_pos_x--, --cursor_pos_y);
            else
                cursor_pos_y = 0;
        }
    }

    // set last printed character to 0
    g_vga_buffer[g_vga_index] = vga_item_entry(0, g_fore_color, g_back_color);
}

// revert back the printed character until n characters
void console_ungetchar_bound(uint8_t n) {
    if (((g_vga_index % VGA_WIDTH) > n) && (n > 0)) {
        g_vga_buffer[g_vga_index--] = vga_item_entry(0, g_fore_color, g_back_color);
        if (cursor_pos_x >= n) {
            vga_set_cursor_pos(cursor_pos_x--, cursor_pos_y);
        } else {
            cursor_pos_x = VGA_WIDTH;
            if (cursor_pos_y > 0)
                vga_set_cursor_pos(cursor_pos_x--, --cursor_pos_y);
            else
                cursor_pos_y = 0;
        }
    }

    // set last printed character to 0
    g_vga_buffer[g_vga_index] = vga_item_entry(0, g_fore_color, g_back_color);
}

void console_gotoxy(uint16_t x, uint16_t y) {
    g_vga_index = (80 * y) + x;
    cursor_pos_x = x;
    cursor_pos_y = y;
    vga_set_cursor_pos(cursor_pos_x, cursor_pos_y);
}

// print string by calling print_char
void console_putstr(const char* str) {
    uint32_t index = 0;
    while (str[index]) {
        write_serial(str[index]);
        if (str[index] == '\n')
            console_newline();
        else
            console_putchar(str[index]);
        index++;
    }
}
