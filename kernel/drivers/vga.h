#pragma once

#include "../../libc/common.h"

#define MAX_ROWS 25
#define MAX_COLS 80

// Screen device I/O ports
#define REG_SCREEN_CTRL 0x3D4
#define REG_SCREEN_DATA 0x3D5

int vga_print_char(char c, int col, int row, char attr);

int vga_handle_scrolling(int cursor_offset);

int vga_get_cursor_offset();

void vga_set_cursor_offset(int offset);

void vga_clear_screen();

enum vga_color {
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

static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) {
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