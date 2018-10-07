#include "vga.h"
#include "ports.h"
#include "../arch/x86/mmu.h"

#include <stdio.h>
#include <string.h>
#include <math.h>

uint16_t *console_fb_addr = NULL;

void *rgb_fb_addr = NULL;
framebuffer_info_t rgb_fb_info = {};

void vga_init(void *fb_addr, uint8_t type, framebuffer_info_t *fb_info) {
    if (type == VGA_FB_TYPE_EGA_TEXT) {
        console_fb_addr = fb_addr;
    } else if (type == VGA_FB_TYPE_RGB) {
        rgb_fb_addr = fb_addr;
        rgb_fb_info = *fb_info;
    }
}

static inline void vga_set_pixel(int x, int y, uint32_t val) {
    *(uint32_t *) (rgb_fb_addr + x * 4 + rgb_fb_info.pitch * y) = val;
}

void vga_fill_rect(int x1, int y1, int x2, int y2, vga_color_t *color) {
    if (rgb_fb_addr == NULL || rgb_fb_info.bpp != 32) return;

    uint32_t val = ((uint32_t) color->red << rgb_fb_info.red_field_position)
                     | ((uint32_t) color->green << rgb_fb_info.green_field_position)
                     | ((uint32_t) color->blue << rgb_fb_info.blue_field_position);

    int xmin = MAX(MIN(x1, x2), 0);
    int xmax = MIN(MAX(x1, x2), rgb_fb_info.width);
    int ymin = MAX(MIN(y1, y2), 0);
    int ymax = MIN(MAX(y1, y2), rgb_fb_info.height);
    for (int y = ymin; y < ymax; y++) {
        for (int x = xmin; x < xmax; x++) {
            vga_set_pixel(x, y, val);
        }
    }
}

void vga_display_image_bgr(int x, int y, BITMAPINFOHEADER *header, uint8_t *image) {
    if (rgb_fb_addr == NULL || rgb_fb_info.bpp != 32) return;

    int xmax = MIN(x + header->biWidth, rgb_fb_info.width);
    int ymax = MIN(y + header->biHeight, rgb_fb_info.height);
    for (int y1 = y; y1 < ymax; y1++) {
        for (int x1 = x; x1 < xmax; x1++) {
            uint8_t *pixel = image + (x1 * y1 * 4); // (header->biHeight - y1)
            uint32_t val = (uint32_t) *(pixel) << rgb_fb_info.blue_field_position
                           | (uint32_t) *(pixel + 1) << rgb_fb_info.green_field_position
                           | (uint32_t) *(pixel + 2) << rgb_fb_info.red_field_position;
            vga_set_pixel(x1, y1, val);
        }
    }
}

int vga_print_char(char c, int col, int row, char attr) {
    if (console_fb_addr == NULL) return 0;
    if (!attr) attr = WHITE_ON_BLACK;

    /* Error control: print a red 'E' if the coords aren't right */
    if (col >= MAX_COLS || row >= MAX_ROWS) {
        console_fb_addr[MAX_COLS * MAX_ROWS - 1] = vga_entry('E', RED_ON_WHITE);
        return vga_get_offset(col, row);
    }

    int offset;
    if (col >= 0 && row >= 0) offset = vga_get_offset(col, row);
    else offset = vga_get_cursor_offset();

    if (c == '\b') {
        offset--;
    } else if (c == '\n') {
        row = vga_get_offset_row(offset);
        offset = vga_get_offset(0, row + 1);
    } else {
        console_fb_addr[offset++] = ((uint16_t) attr << 8 | c);
    }
    vga_set_cursor_offset(offset = vga_handle_scrolling(offset));
    return offset;
}

int vga_get_cursor_offset() {
    if (console_fb_addr == NULL) return 0;
    port_byte_out(REG_SCREEN_CTRL, 14);
    int offset = port_byte_in(REG_SCREEN_DATA) << 8; /* High byte: << 8 */
    port_byte_out(REG_SCREEN_CTRL, 15);
    offset += port_byte_in(REG_SCREEN_DATA);
    return offset; /* Position * size of character cell */
}

void vga_set_cursor_offset(int offset) {
    if (console_fb_addr == NULL) return;
    port_byte_out(REG_SCREEN_CTRL, 14);
    port_byte_out(REG_SCREEN_DATA, (unsigned char) (offset >> 8));
    port_byte_out(REG_SCREEN_CTRL, 15);
    port_byte_out(REG_SCREEN_DATA, (unsigned char) (offset & 0xff));
}

void vga_clear_screen() {
    if (console_fb_addr == NULL) return;
    memset((void *) console_fb_addr, 0, MAX_COLS * MAX_ROWS * sizeof(*console_fb_addr));
    vga_set_cursor_offset(vga_get_offset(0, 0));
}

/* Advance the text cursor, scrolling the video buffer if necessary. */
int vga_handle_scrolling(int cursor_offset) {
    if (console_fb_addr == NULL) return 0;
    if (cursor_offset < MAX_ROWS * MAX_COLS)
        return cursor_offset;

    /* Shuffle the rows back one. */
    for (int i = 1; i < MAX_ROWS; i++) {
        memcpy((void *) (console_fb_addr + vga_get_offset(0, i - 1)),
               (void *) (console_fb_addr + vga_get_offset(0, i)),
               MAX_COLS * sizeof(*console_fb_addr));
    }

    /* Blank the last line by setting all bytes to 0 */
    memset((void *) (console_fb_addr + vga_get_offset(0, MAX_ROWS - 1)),
           0, MAX_COLS * sizeof(*console_fb_addr));

    cursor_offset -= MAX_COLS;
    return cursor_offset;
}