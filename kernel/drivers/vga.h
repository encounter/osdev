#pragma once

#include <common.h>
#include "../bmp.h"

#define MAX_ROWS 25
#define MAX_COLS 80

// Screen device I/O ports
#define REG_SCREEN_CTRL 0x3D4
#define REG_SCREEN_DATA 0x3D5

#define VGA_FB_TYPE_INDEXED 0
#define VGA_FB_TYPE_RGB     1
#define VGA_FB_TYPE_EGA_TEXT     2

typedef struct framebuffer_info {
    uint32_t pitch;
    uint32_t width;
    uint32_t height;
    uint8_t bpp;
    uint32_t palette_addr;
    uint16_t palette_num_colors;
    uint8_t red_field_position;
    uint8_t red_mask_size;
    uint8_t green_field_position;
    uint8_t green_mask_size;
    uint8_t blue_field_position;
    uint8_t blue_mask_size;
} framebuffer_info_t;

typedef struct vga_rgb_color {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} vga_rgb_color_t;

void vga_init(void *fb_addr, uint8_t type, framebuffer_info_t *fb_info);

int vga_load_font(const char *filename);

void vga_fill_rect(int x1, int y1, int x2, int y2, vga_rgb_color_t *color);

void vga_display_image_bgra(int x, int y, bmp_info_header_t *header, uint8_t *image);

// --- Console

int vga_print_char(char c, int offset, char attr);

int vga_handle_scrolling(int cursor_offset);

int vga_get_cursor_offset();

void vga_set_cursor_offset(int offset);

void vga_clear_screen();

enum vga_text_color {
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_BLUE = 1,
    VGA_COLOR_GREEN = 2,
    VGA_COLOR_CYAN = 3,
    VGA_COLOR_RED = 4,
    VGA_COLOR_MAGENTA = 5,
    VGA_COLOR_BROWN = 6,
    VGA_COLOR_LIGHT_GREY = 7,
    VGA_COLOR_DARK_GREY = 8,
    VGA_COLOR_LIGHT_BLUE = 9,
    VGA_COLOR_LIGHT_GREEN = 10,
    VGA_COLOR_LIGHT_CYAN = 11,
    VGA_COLOR_LIGHT_RED = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_LIGHT_BROWN = 14,
    VGA_COLOR_WHITE = 15,
};
typedef enum vga_text_color vga_text_color_t;

static inline uint8_t vga_entry_color(vga_text_color_t fg, vga_text_color_t bg) {
    return fg | bg << 4;
}

// Attribute byte for our default colour scheme.
#define WHITE_ON_BLACK (vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK))
#define RED_ON_BLACK (vga_entry_color(VGA_COLOR_RED, VGA_COLOR_BLACK))
#define RED_ON_WHITE (vga_entry_color(VGA_COLOR_RED, VGA_COLOR_WHITE))

static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
    return (uint16_t) uc | (uint16_t) color << 8;
}

static inline int vga_get_offset(int col, int row) {
    return row * MAX_COLS + col;
}

static inline int vga_get_offset_row(int offset) {
    return offset / MAX_COLS;
}

static inline int vga_get_offset_col(int offset) {
    return offset - (vga_get_offset_row(offset) * MAX_COLS);
}