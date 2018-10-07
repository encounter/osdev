#include "console.h"
#include "drivers/vga.h"
#include "drivers/serial.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

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
static void knprint(const char *str, size_t len, uint8_t vga_color) {
    if (vga_enabled) {
        int offset = -1;
        for (size_t i = 0; i < len; ++i) {
            offset = vga_print_char(str[i], offset, vga_color);
        }
    }

    if (serial_enabled) {
        for (size_t i = 0; i < len; ++i) {
            serial_write(str[i]);
        }
    }
}

void clear_screen() {
    if (vga_enabled) {
        vga_clear_screen();
    }
}

_noreturn
void panic(char *str, ...) {
    vga_enabled = false;

    va_list args;
    va_start(args, str);
    if (str != NULL) vfprintf(stderr, str, args);
    va_end(args);
    fflush(stderr);

    // Spin lock
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

// --- stdio

int errno;

size_t __stdout_write(FILE *f, const char *str, size_t len) {
    size_t rem = f->wpos - f->wbase;
    if (rem) knprint(f->wbase, rem, WHITE_ON_BLACK);
    if (len) knprint(str, len, WHITE_ON_BLACK);

    f->wend = f->buf + f->buf_size;
    f->wpos = f->wbase = f->buf;
    return len;
}

size_t __stderr_write(FILE *f, const char *str, size_t len) {
    size_t rem = f->wpos - f->wbase;
    if (rem) knprint(f->wbase, rem, RED_ON_BLACK);
    if (len) knprint(str, len, RED_ON_BLACK);

    f->wend = f->buf + f->buf_size;
    f->wpos = f->wbase = f->buf;
    return len;
}

off_t __stdio_seek(FILE *file, const off_t offset, int origin) {
    return 0; // TODO
}

int __stdio_close(FILE *file) {
    return 0; // TODO
}

static char __stdout_buf[BUFSIZ + UNGET];
FILE *stdout = &(FILE) {
        .buf = __stdout_buf + UNGET,
        .buf_size = BUFSIZ,
        .fd = 1,
        .flags = F_PERM | F_NORD,
        .lbf = '\n',
        .write = __stdout_write,
        .seek = __stdio_seek,
        .close = __stdio_close,
        .lock = -1,
};

static char __stderr_buf[BUFSIZ + UNGET];
FILE *stderr = &(FILE) {
        .buf = __stderr_buf + UNGET,
        .buf_size = BUFSIZ,
        .fd = 1,
        .flags = F_PERM | F_NORD,
        .lbf = '\n',
        .write = __stderr_write,
        .seek = __stdio_seek,
        .close = __stdio_close,
        .lock = -1,
};