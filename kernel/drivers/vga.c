#include "vga.h"
#include "ports.h"
#include "../arch/x86/mmu.h"

#include <stdio.h>
#include <string.h>

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

void vga_fill_color(uint8_t r, uint8_t g, uint8_t b) {
    if (rgb_fb_addr == NULL || rgb_fb_info.bpp != 32) return;

    uint32_t color = ((uint32_t) r << rgb_fb_info.red_field_position)
            | ((uint32_t) g << rgb_fb_info.green_field_position)
            | ((uint32_t) b << rgb_fb_info.blue_field_position);

    for (int i = 0; i < rgb_fb_info.width / 2 && i < rgb_fb_info.height / 2; i++) {
        uint32_t *pixel = rgb_fb_addr + rgb_fb_info.pitch * i;
        printf("Setting pixel %Xh to %Xh\n", (uintptr_t) pixel, color);
        *pixel = color;
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