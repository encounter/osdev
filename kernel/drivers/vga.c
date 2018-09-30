#include "vga.h"
#include "ports.h"

#include <string.h>

#define VIDEO_ADDRESS ((uint16_t *) 0xC00B8000)

/**********************************************************
 * Private kernel functions                               *
 **********************************************************/

/**
 * Innermost print function for our kernel, directly accesses the video memory 
 *
 * If 'col' and 'row' are negative, we will print at current cursor location
 * If 'attr' is zero it will use 'white on black' as default
 * Returns the offset of the next character
 * Sets the video cursor to the returned offset
 */
int vga_print_char(char c, int col, int row, char attr) {
    if (!attr) attr = WHITE_ON_BLACK;

    /* Error control: print a red 'E' if the coords aren't right */
    if (col >= MAX_COLS || row >= MAX_ROWS) {
        VIDEO_ADDRESS[MAX_COLS * MAX_ROWS - 1] = vga_entry('E', RED_ON_WHITE);
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
        VIDEO_ADDRESS[offset++] = (uint16_t) (attr << 8 | c);
    }
    vga_set_cursor_offset(offset);
    return offset;
}

int vga_get_cursor_offset() {
    /*
     * Use the VGA ports to get the current cursor position
     * 1. Ask for high byte of the cursor offset (data 14)
     * 2. Ask for low byte (data 15)
     */
    port_byte_out(REG_SCREEN_CTRL, 14);
    int offset = port_byte_in(REG_SCREEN_DATA) << 8; /* High byte: << 8 */
    port_byte_out(REG_SCREEN_CTRL, 15);
    offset += port_byte_in(REG_SCREEN_DATA);
    return offset; /* Position * size of character cell */
}

void vga_set_cursor_offset(int offset) {
    /* Similar to get_cursor_offset, but instead of reading we write data */
    port_byte_out(REG_SCREEN_CTRL, 14);
    port_byte_out(REG_SCREEN_DATA, (unsigned char) (offset >> 8));
    port_byte_out(REG_SCREEN_CTRL, 15);
    port_byte_out(REG_SCREEN_DATA, (unsigned char) (offset & 0xff));
}

void vga_clear_screen() {
    memset((void *) VIDEO_ADDRESS, 0, MAX_COLS * MAX_ROWS * sizeof(*VIDEO_ADDRESS));
    vga_set_cursor_offset(vga_get_offset(0, 0));
}

/* Advance the text cursor, scrolling the video buffer if necessary. */
int vga_handle_scrolling(int cursor_offset) {
    // If the cursor is within the screen, return it unmodified. 
    if (cursor_offset < MAX_ROWS * MAX_COLS)
        return cursor_offset;

    /* Shuffle the rows back one. */
    for (int i = 1; i < MAX_ROWS; i++) {
        memcpy((void *) (VIDEO_ADDRESS + vga_get_offset(0, i - 1)),
               (void *) (VIDEO_ADDRESS + vga_get_offset(0, i)),
               MAX_COLS * sizeof(*VIDEO_ADDRESS));
    }

    /* Blank the last line by setting all bytes to 0 */
    memset((void *) (VIDEO_ADDRESS + vga_get_offset(0, MAX_ROWS - 1)),
           0, MAX_COLS * sizeof(*VIDEO_ADDRESS));

    // Move the offset back one row, such that it is now on the last 
    // row, rather than off the edge of the screen.
    cursor_offset -= MAX_COLS;
    // Return the updated cursor position.
    return cursor_offset;
}