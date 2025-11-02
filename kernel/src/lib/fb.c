#include "kernel/lib/fb.h"

fb_t fb;

/**
 * @brief Initializes the framebuffer using information from the bootloader.
 *
 * @param boot Pointer to the multiboot2 structure provided by the bootloader.
 */
void init_fb(mb2_t* boot) {
    mb2_tag_fb_t* fb_info = (mb2_tag_fb_t*) mb2_find_tag(boot, MB2_TAG_FB);
    fb.address = (uintptr_t) fb_info->addr;
    fb.pitch = fb_info->pitch;
    fb.width = fb_info->width;
    fb.height = fb_info->height;
    fb.bpp = fb_info->bpp;
}

/**
 * @brief Retrieves the current framebuffer information.
 *
 * @return The current framebuffer structure.
 */
fb_t get_fb() {
    return fb;
}

/**
 * @brief Draws a pixel at the specified coordinates with the given color.
 *
 * @param x The x-coordinate of the pixel.
 * @param y The y-coordinate of the pixel.
 * @param c The color of the pixel in ARGB format.
 */
void draw_pixel(int x, int y, uint32_t c) {
    int index = x + (y * fb.pitch / 4);
    ((uint32_t*) fb.address)[index] = c;
}

/**
 * @brief Draws a horizontal line starting from the specified coordinates with the given width and color.
 *
 * @param x The starting x-coordinate of the line.
 * @param y The y-coordinate of the line.
 * @param w The width of the line.
 * @param c The color of the line in ARGB format.
 */
void draw_line_hor(int x, int y, int w, uint32_t c) {
    int p = x + y * fb.pitch / 4;
    for (int i = 0; i < w; i++)
        ((uint32_t*) fb.address)[p++] = c;
}

/**
 * @brief Draws a vertical line starting from the specified coordinates with the given height and color.
 *
 * @param x The x-coordinate of the line.
 * @param y The starting y-coordinate of the line.
 * @param h The height of the line.
 * @param c The color of the line in ARGB format.
 */
void draw_line_ver(int x, int y, int h, uint32_t c) {
    int p = x + y * fb.pitch / 4;
    for (int i = 0; i < h; i++)
        ((uint32_t*) fb.address)[p += fb.pitch / 4] = c;
}

/**
 * @brief Converts an ARGB structure to a 32-bit color value.
 *
 * @param c Pointer to the ARGB structure.
 * @return The 32-bit color value.
 */
uint32_t argb_to_color(argb_t* c) {
    return (c->a << 24 | c->r << 16 | c->g << 8 | c->b << 0);
}

/**
 * @brief Converts a VGA color index to a 32-bit color value.
 *
 * @param vga The VGA color index.
 * @return The 32-bit color value.
 */
uint32_t vga_to_color(uint8_t vga) {
    switch (vga) {
    case 0:
        return 0xff000000;
    case 1:
        return 0xff0000ff;
    case 2:
        return 0xff00ff00;
    case 3:
        return 0xff00ffff;
    case 4:
        return 0xffff0000;
    case 5:
        return 0xffff00ff;
    case 6:
        return 0xff8b4513;
    case 7:
        return 0xffd3d3d3;
    case 8:
        return 0xffa9a9a9;
    case 9:
        return 0xff00bfff;
    case 10:
        return 0xff7cfc00;
    case 11:
        return 0xffe0ffff;
    case 12:
        return 0xfff08080;
    case 13:
        return 0xffff80ff;
    case 14:
        return 0xffcd8032;
    case 15:
        return 0xffffffff;
    default:
        return 0xff000000;
    }
}