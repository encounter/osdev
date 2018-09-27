#include "console.h"
#include "drivers/vga.h"
#include "drivers/serial.h"

static bool vga_enabled = false;
static bool serial_enabled = false;

bool console_vga_enabled() {
    return vga_enabled;
}

void console_set_vga_enabled(bool enabled) {
    vga_enabled = enabled;
}

bool console_serial_enabled() {
    return serial_enabled;
}

void console_set_serial_enabled(bool enabled) {
    serial_enabled = enabled;
}

/**
 * Print a message on the specified location
 * If col, row, are negative, we will use the current offset
 */

void kprint(const char *message) {
    if (vga_enabled) {
        int offset = vga_get_cursor_offset();
        int row = vga_get_offset_row(offset);
        int col = vga_get_offset_col(offset);

        int i = 0;
        while (message[i] != 0) {
            offset = vga_print_char(message[i++], col, row, vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
            row = vga_get_offset_row(offset);
            col = vga_get_offset_col(offset);
        }
    }

    if (serial_enabled) {
        int i = 0;
        while (message[i] != 0) {
            serial_write(message[i++]);
        }
    }
}

void kprint_char(char c) {
    if (vga_enabled) {
        int offset = vga_get_cursor_offset();
        vga_print_char(c, vga_get_offset_col(offset), vga_get_offset_row(offset), WHITE_ON_BLACK);
    }

    if (serial_enabled) {
        serial_write(c);
    }
}

void kprint_backspace() {
    if (vga_enabled) {
        int offset = vga_get_cursor_offset() - 1;
        vga_print_char(' ', vga_get_offset_col(offset), vga_get_offset_row(offset), WHITE_ON_BLACK);
        vga_set_cursor_offset(offset);
    }

    if (serial_enabled) {
        serial_write(0x8);
        serial_write(' ');
        serial_write(0x8);
    }
}

void clear_screen() {
    if (vga_enabled) {
        vga_clear_screen();
    }
}

// FIXME get rid of evil macros
#define to_hex(c) ((char) ((c) > 9 ? (c) + 0x37 : (c) + 0x30))
#define phexchar(shr) c = val >> (shr) & 0xF; if (c || i > 2 || !shr) hextemp[i++] = to_hex(c)
char hextemp[14] = {'0', 'x'};

void kprint_uint64(uint64_t val) {
    uint8_t i = 2, c;
    phexchar(44);
    phexchar(40);
    phexchar(36);
    phexchar(32);
    phexchar(28);
    phexchar(24);
    phexchar(20);
    phexchar(16);
    phexchar(12);
    phexchar(8);
    phexchar(4);
    phexchar(0);
    hextemp[i] = 0;
    kprint(hextemp);
}

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
    hextemp[i] = 0;
    kprint(hextemp);
}

void kprint_uint16(uint16_t val) {
    uint8_t i = 2, c;
    phexchar(12);
    phexchar(8);
    phexchar(4);
    phexchar(0);
    hextemp[i] = 0;
    kprint(hextemp);
}

void kprint_uint8(uint8_t val) {
    uint8_t i = 2, c;
    phexchar(4);
    phexchar(0);
    hextemp[i] = 0;
    kprint(hextemp);
}

_noreturn
void panic(char *str) {
    if (str != NULL) kprint(str);
    __asm__("cli");
    while (1) __asm__("hlt");
}

#if UINT32_MAX == UINTPTR_MAX
#define STACK_CHK_GUARD 0xe2dee396
#else
#define STACK_CHK_GUARD 0x595e9fbd94fda766
#endif

_unused
        uintptr_t __stack_chk_guard = STACK_CHK_GUARD;

_noreturn _unused
void __stack_chk_fail() {
    panic("Stack smashing detected");
}