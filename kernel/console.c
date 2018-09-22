#include "console.h"
#include "drivers/ports.h"

#include <string.h>

int handle_scrolling(int cursor_offset);

int get_cursor_offset();

void set_cursor_offset(int offset);

int print_char(char c, int col, int row, char attr);

static inline int get_offset(int col, int row) {
    return handle_scrolling(row * MAX_COLS + col);
}

static inline int get_offset_row(int offset) {
    return offset / MAX_COLS;
}

static inline int get_offset_col(int offset) {
    return offset - (get_offset_row(offset) * MAX_COLS);
}

/**********************************************************
 * Public Kernel API functions                            *
 **********************************************************/

/**
 * Print a message on the specified location
 * If col, row, are negative, we will use the current offset
 */
void kprint_at(char *message, int col, int row) {
    int offset;
    if (col < 0 || row < 0) {
        offset = get_cursor_offset();
        row = get_offset_row(offset);
        col = get_offset_col(offset);
    }

    int i = 0;
    while (message[i] != 0) {
        offset = print_char(message[i++], col, row, WHITE_ON_BLACK);
        row = get_offset_row(offset);
        col = get_offset_col(offset);
    }
}

void kprint(char *message) {
    kprint_at(message, -1, -1);
}

void kprint_char(char c) {
    int offset = get_cursor_offset();
    print_char(c, get_offset_col(offset), get_offset_row(offset), WHITE_ON_BLACK);
}

// FIXME get rid of evil macros
#define to_hex(c) ((char) ((c) > 9 ? (c) + 0x37 : (c) + 0x30))
#define phexchar(shr) c = val >> (shr) & 0xF; if (c || i > 2 || !shr) hextemp[i++] = to_hex(c)
char hextemp[10] = {'0', 'x'};

void kprint_uint32(uint32_t val) {
    uint8_t i = 2, c;
    phexchar(28);
    phexchar(24);
    phexchar(20);
    phexchar(16);
    phexchar(12);
    phexchar(8);
    phexchar(4);
    phexchar(0);
    if (i < 10) hextemp[i] = 0;
    kprint(hextemp);
}


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
int print_char(char c, int col, int row, char attr) {
    if (!attr) attr = WHITE_ON_BLACK;

    /* Error control: print a red 'E' if the coords aren't right */
    if (col >= MAX_COLS || row >= MAX_ROWS) {
        VIDEO_ADDRESS[MAX_COLS * MAX_ROWS - 1] = (RED_ON_WHITE << 8) | 'E';
        return get_offset(col, row);
    }

    int offset;
    if (col >= 0 && row >= 0) offset = get_offset(col, row);
    else offset = get_cursor_offset();

    if (c == '\n') {
        row = get_offset_row(offset);
        offset = get_offset(0, row + 1);
    } else {
        VIDEO_ADDRESS[offset++] = (uint16_t) (attr << 8 | c);
    }
    set_cursor_offset(offset);
    return offset;
}

int get_cursor_offset() {
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

void set_cursor_offset(int offset) {
    /* Similar to get_cursor_offset, but instead of reading we write data */
    port_byte_out(REG_SCREEN_CTRL, 14);
    port_byte_out(REG_SCREEN_DATA, (unsigned char) (offset >> 8));
    port_byte_out(REG_SCREEN_CTRL, 15);
    port_byte_out(REG_SCREEN_DATA, (unsigned char) (offset & 0xff));
}

void clear_screen() {
    memset((void *) VIDEO_ADDRESS, 0, MAX_COLS * MAX_ROWS * sizeof(*VIDEO_ADDRESS));
    set_cursor_offset(get_offset(0, 0));
}

/* Advance the text cursor, scrolling the video buffer if necessary. */
int handle_scrolling(int cursor_offset) {
    // If the cursor is within the screen, return it unmodified. 
    if (cursor_offset < MAX_ROWS * MAX_COLS)
        return cursor_offset;

    /* Shuffle the rows back one. */
    for (int i = 1; i < MAX_ROWS; i++) {
        memcpy((void *) (VIDEO_ADDRESS + get_offset(0, i - 1)),
               (void *) (VIDEO_ADDRESS + get_offset(0, i)),
               MAX_COLS * sizeof(*VIDEO_ADDRESS));
    }

    /* Blank the last line by setting all bytes to 0 */
    memset((void *) (VIDEO_ADDRESS + get_offset(0, MAX_ROWS - 1)),
           0, MAX_COLS * sizeof(*VIDEO_ADDRESS));

    // Move the offset back one row, such that it is now on the last 
    // row, rather than off the edge of the screen.
    cursor_offset -= MAX_COLS;
    // Return the updated cursor position.
    return cursor_offset;
}
