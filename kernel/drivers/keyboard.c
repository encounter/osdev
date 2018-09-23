#include "ports.h"
#include "timer.h"
#include "../console.h"
#include "../isr.h"

#include <stdbool.h>
#include <string.h>
#include <malloc.h>
#include <math.h>

typedef void (*shell_callback)(char *input);

static shell_callback shell_cb = NULL;

#define BACKSPACE 0x0E
#define ENTER 0x1C
#define L_SHIFT 0x2A
#define R_SHIFT 0x36
#define L_SHIFT_RELEASE 0xAA
#define R_SHIFT_RELEASE 0xB6

#define KEY_BUFFER_INITIAL_SIZE 0x100
static char *key_buffer;
static size_t key_buffer_size;
static size_t key_buffer_used;

#define ASCII_MAX 58
const char *sc_name[ASCII_MAX] = {
        "ERROR", "Esc", "1", "2", "3", "4", "5", "6",
        "7", "8", "9", "0", "-", "=", "Backspace", "Tab", "Q", "W", "E",
        "R", "T", "Y", "U", "I", "O", "P", "[", "]", "Enter", "Lctrl",
        "A", "S", "D", "F", "G", "H", "J", "K", "L", ";", "'", "`",
        "LShift", "\\", "Z", "X", "C", "V", "B", "N", "M", ",", ".",
        "/", "RShift", "Keypad *", "LAlt", "Spacebar"
};
const char sc_ascii[ASCII_MAX] = {
        0, 0, '!', '@', '#', '$', '%', '^',
        '&', '*', '(', ')', '_', '+', 0, 0, 'Q', 'W', 'E', 'R', 'T', 'Y',
        'U', 'I', 'O', 'P', '{', '}', 0, 0, 'A', 'S', 'D', 'F', 'G',
        'H', 'J', 'K', 'L', ':', '"', '~', 0, '\\', 'Z', 'X', 'C', 'V',
        'B', 'N', 'M', '<', '>', '?', 0, 0, 0, ' '
};
const char sc_ascii_lower[ASCII_MAX] = {
        0, 0, '1', '2', '3', '4', '5', '6',
        '7', '8', '9', '0', '-', '=', 0, 0, 'q', 'w', 'e', 'r', 't', 'y',
        'u', 'i', 'o', 'p', '[', ']', 0, 0, 'a', 's', 'd', 'f', 'g',
        'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\', 'z', 'x', 'c', 'v',
        'b', 'n', 'm', ',', '.', '/', 0, 0, 0, 0
};

static bool shift_pressed = false;

static bool key_buffer_append(const char c) {
    if (key_buffer == NULL) {
        key_buffer = malloc(key_buffer_size = KEY_BUFFER_INITIAL_SIZE);
        if (key_buffer == NULL) return false;
    } else if (key_buffer_size + 1 <= key_buffer_used) {
        key_buffer = realloc(key_buffer, key_buffer_size += KEY_BUFFER_INITIAL_SIZE);
        if (key_buffer == NULL) return false;
    }

    key_buffer[key_buffer_used++] = c;
    key_buffer[key_buffer_used] = '\0';
    return true;
}

static void key_buffer_clear() {
    key_buffer[key_buffer_used = 0] = '\0';

    // Shrink key_buffer if it expanded
    if (key_buffer_size > KEY_BUFFER_INITIAL_SIZE) {
        key_buffer = realloc(key_buffer, key_buffer_size = KEY_BUFFER_INITIAL_SIZE);
    }
}

static void irq_callback(__attribute__((unused)) registers_t regs) {
    unsigned char c = port_byte_in(0x60);
    if (c == L_SHIFT || c == R_SHIFT) {
        shift_pressed = true;
    } else if (c == L_SHIFT_RELEASE || c == R_SHIFT_RELEASE) {
        shift_pressed = false;
    } else if (c == BACKSPACE) {
        if (key_buffer_used > 0) {
            key_buffer[--key_buffer_used] = '\0';
            kprint_backspace();
        }
    } else if (c == ENTER) {
        if (shell_cb != NULL) shell_cb(key_buffer);
        key_buffer_clear();
    }

    if (c > ASCII_MAX) {
        return;
    } else if (!shift_pressed && sc_ascii_lower[c]) {
        if (key_buffer_append(sc_ascii_lower[c])) {
            kprint_char(sc_ascii_lower[c]);
        }
    } else if (sc_ascii[c]) {
        if (key_buffer_append(sc_ascii[c])) {
            kprint_char(sc_ascii[c]);
        }
    } else {
//        kprint("keypress ");
//        kprint_uint32(c);
//        kprint(" @ ");
//        kprint_uint32(get_tick());
//        kprint_char('\n');
    }
}

void init_keyboard(shell_callback cb) {
    register_interrupt_handler(IRQ1, &irq_callback);
    shell_cb = cb; // Having this shell logic in here is wrong, but...
}

void key_buffer_set(char *input) {
    key_buffer_used = strlen(input);
    key_buffer = realloc(key_buffer, max(key_buffer_used, KEY_BUFFER_INITIAL_SIZE));
    if (key_buffer == NULL) return; // return error of some sort?
    strncpy(key_buffer, input, key_buffer_used);
}