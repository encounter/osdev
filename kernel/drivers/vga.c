#include "vga.h"
#include "ports.h"
#include "../psf.h"
#include "../console.h"

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <malloc.h>

// EGA text console
uint16_t *console_fb_addr = NULL;

// RGB text console
void *rgb_fb_addr = NULL;
framebuffer_info_t rgb_fb_info = {};

psf_font_t rgb_console_font = {};
void *rgb_console_font_glyphs = NULL;

unsigned int rgb_console_rows = 0;
unsigned int rgb_console_cols = 0;
int rgb_console_offset = 0;

typedef struct rgb_console_buffer_entry {
    char c;
    vga_rgb_color_t fg_color;
    vga_rgb_color_t bg_color;
} rgb_console_buffer_entry_t;

rgb_console_buffer_entry_t *rgb_console_buffer = NULL;

static void vga_console_repaint();

void vga_init(void *fb_addr, uint8_t type, framebuffer_info_t *fb_info) {
    if (type == VGA_FB_TYPE_EGA_TEXT) {
        console_fb_addr = fb_addr;
    } else if (type == VGA_FB_TYPE_RGB) {
        rgb_fb_addr = fb_addr;
        rgb_fb_info = *fb_info;
    }
}

int vga_load_font(const char *filename) {
    int prev_width = 0, prev_height = 0;
    if (rgb_console_font_glyphs != NULL) {
        prev_width = rgb_console_font.width;
        prev_height = rgb_console_font.height;
        free(rgb_console_font_glyphs);
    }

    rgb_console_font_glyphs = psf_read_font(filename, &rgb_console_font);
    if (rgb_console_font_glyphs == NULL) {
        fprintf(stderr, "Failed to read font %s (err: %d)\n", filename, errno);
        return -1;
    }
    rgb_console_cols = rgb_fb_info.width / rgb_console_font.width;
    rgb_console_rows = rgb_fb_info.height / rgb_console_font.height;

    size_t buffer_size = rgb_console_rows * rgb_console_cols * sizeof(rgb_console_buffer_entry_t);
    rgb_console_buffer_entry_t *new_buffer = malloc(buffer_size);
    if (new_buffer == NULL) {
        fprintf(stderr, "Failed to allocate memory for console buffer.\n");
        return -1;
    }
    memset(new_buffer, 0, buffer_size);

    if (rgb_console_buffer != NULL) {
        size_t prev_size = (rgb_fb_info.width / prev_width)
                           * (rgb_fb_info.height / prev_height)
                           * sizeof(rgb_console_buffer_entry_t);
        if (prev_size > buffer_size) {
            size_t diff = prev_size - buffer_size;
            memcpy(new_buffer, (void *) rgb_console_buffer + diff, buffer_size);
            // FIXME refactor to avoid this kludgy division
            rgb_console_offset -= diff / sizeof(rgb_console_buffer_entry_t);
        } else {
            memcpy(new_buffer, rgb_console_buffer, prev_size);
            memset((void *) new_buffer + prev_size, 0, buffer_size - prev_size);
        }
        free(rgb_console_buffer);
    }
    rgb_console_buffer = new_buffer;
    vga_console_repaint();
    return 0;
}

static inline void vga_set_pixel(int x, int y, uint32_t val) {
    *(uint32_t *) (rgb_fb_addr + x * 4 + rgb_fb_info.pitch * y) = val;
}

void vga_fill_rect(int x1, int y1, int x2, int y2, vga_rgb_color_t *color) {
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

void vga_display_image_bgra(int x, int y, bmp_info_header_t *header, uint8_t *image) {
    if (rgb_fb_addr == NULL || rgb_fb_info.bpp != 32) return;

    int xmax = MIN(x + header->width, rgb_fb_info.width);
    int ymax = MIN(y + header->height, rgb_fb_info.height);
    for (int y1 = y; y1 < ymax; y1++) {
        for (int x1 = x; x1 < xmax; x1++) {
            uint8_t *pixel = image + ((x1 + ((header->height - y1) * header->width)) * 4);
            vga_set_pixel(x1, y1, (uint32_t) *pixel << rgb_fb_info.blue_field_position
                                  | (uint32_t) *(pixel + 1) << rgb_fb_info.green_field_position
                                  | (uint32_t) *(pixel + 2) << rgb_fb_info.red_field_position);
        }
    }
}

static void vga_print_glyph(int offset, char c, vga_rgb_color_t *fg_color, vga_rgb_color_t *bg_color) {
    if (rgb_fb_addr == NULL || rgb_fb_info.bpp != 32) return;

    int row = offset / rgb_console_cols;
    int x = (offset % rgb_console_cols) * rgb_console_font.width;
    int y = row * rgb_console_font.height;

    uint32_t fg_val = ((uint32_t) fg_color->red << rgb_fb_info.red_field_position)
                      | ((uint32_t) fg_color->green << rgb_fb_info.green_field_position)
                      | ((uint32_t) fg_color->blue << rgb_fb_info.blue_field_position);
    uint32_t bg_val = ((uint32_t) bg_color->red << rgb_fb_info.red_field_position)
                      | ((uint32_t) bg_color->green << rgb_fb_info.green_field_position)
                      | ((uint32_t) bg_color->blue << rgb_fb_info.blue_field_position);

    int xmax = MIN(x + rgb_console_font.width, rgb_fb_info.width);
    int ymax = MIN(y + rgb_console_font.height, rgb_fb_info.height);
    uint8_t *glyph = rgb_console_font_glyphs + (rgb_console_font.bytes_per_glyph * (uint8_t) c);

    for (int y1 = y; y1 < ymax; y1++) {
        for (int x1 = x; x1 < xmax; x1++) {
            uint8_t gb = *(glyph + (y1 - y));
            vga_set_pixel(x1, y1, (gb >> (8 - (x1 - x))) & 1 ? fg_val : bg_val);
        }
    }
}

int vga_print_char(char c, int offset, char attr) {
    if (rgb_fb_addr != NULL) {
        if (rgb_console_font_glyphs == NULL) return 0;
        if (offset < 0) {
            offset = rgb_console_offset;
        } else if (offset >= rgb_console_rows * rgb_console_cols) {
            return rgb_console_offset;
        }

        if (c == '\b') {
            offset--;
        } else if (c == '\n') {
            unsigned int row = offset / rgb_console_cols;
            offset = (row + 1) * rgb_console_cols;
        } else {
            vga_rgb_color_t fg_color = {255, 255, 255};
            vga_rgb_color_t bg_color = {0, 0, 0};

            // FIXME make more dynamic
            if (attr == RED_ON_BLACK) {
                fg_color = (vga_rgb_color_t) {255, 0, 0};
                bg_color = (vga_rgb_color_t) {0, 0, 0};
            }

            rgb_console_buffer[offset] = (rgb_console_buffer_entry_t) {c, fg_color, bg_color};
            if (!c) c = ' ';
            vga_print_glyph(offset++, c, &fg_color, &bg_color);
        }
        rgb_console_offset = offset = vga_handle_scrolling(offset);
    } else if (console_fb_addr != NULL) {
        if (!attr) attr = WHITE_ON_BLACK;

        if (offset < 0) {
            offset = vga_get_cursor_offset();
        } else if (offset >= MAX_ROWS * MAX_COLS) {
            console_fb_addr[MAX_COLS * MAX_ROWS - 1] = vga_entry('E', RED_ON_WHITE);
            return vga_get_cursor_offset();
        }

        if (c == '\b') {
            offset--;
        } else if (c == '\n') {
            offset = vga_get_offset(0, vga_get_offset_row(offset) + 1);
        } else {
            console_fb_addr[offset++] = ((uint16_t) attr << 8 | c);
        }
        vga_set_cursor_offset(offset = vga_handle_scrolling(offset));
    }

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
    if (console_fb_addr != NULL) {
        memset(console_fb_addr, 0, MAX_COLS * MAX_ROWS * sizeof(*console_fb_addr));
        vga_set_cursor_offset(vga_get_offset(0, 0));
    } else if (rgb_fb_addr != NULL) {
        vga_fill_rect(0, 0, rgb_fb_info.width, rgb_fb_info.height, &(vga_rgb_color_t) {0, 0, 0});
        memset(rgb_console_buffer, 0, rgb_console_rows * rgb_console_cols);
        rgb_console_offset = 0;
    }
}

static void vga_console_repaint() {
    for (size_t i = 0; i < rgb_console_rows * rgb_console_cols; ++i) {
        rgb_console_buffer_entry_t *entry = &rgb_console_buffer[i];
        char c = entry->c;
        if (!c) c = ' '; // FIXME improve
        vga_print_glyph(i, c, &entry->fg_color, &entry->bg_color);
    }
}

// Advance the text cursor, scrolling the video buffer if necessary.
int vga_handle_scrolling(int cursor_offset) {
    if (console_fb_addr != NULL) {
        if (cursor_offset < MAX_ROWS * MAX_COLS)
            return cursor_offset;

        for (int i = 1; i < MAX_ROWS; i++) {
            memcpy(console_fb_addr + vga_get_offset(0, i - 1),
                   console_fb_addr + vga_get_offset(0, i),
                   MAX_COLS * sizeof(*console_fb_addr));
        }

        memset(console_fb_addr + vga_get_offset(0, MAX_ROWS - 1),
               0, MAX_COLS * sizeof(*console_fb_addr));

        cursor_offset -= MAX_COLS;
    } else if (rgb_fb_addr != NULL) {
        if (cursor_offset < rgb_console_rows * rgb_console_cols)
            return cursor_offset;

        size_t end_offset = (rgb_console_rows - 1) * rgb_console_cols;
        memmove(rgb_console_buffer, rgb_console_buffer + rgb_console_cols,
                end_offset * sizeof(rgb_console_buffer_entry_t));
        memset(rgb_console_buffer + end_offset, 0, rgb_console_cols * sizeof(rgb_console_buffer_entry_t));
        cursor_offset -= rgb_console_cols;
        vga_console_repaint();
    }

    return cursor_offset;
}