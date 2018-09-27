#include "keyboard.h"
#include "ports.h"
#include "timer.h"
#include "../console.h"
#include "../isr.h"
#include "../shell.h"

#include <string.h>

#define BACKSPACE 0x0E
#define ENTER 0x1C
#define L_SHIFT 0x2A
#define R_SHIFT 0x36
#define L_SHIFT_RELEASE 0xAA
#define R_SHIFT_RELEASE 0xB6
#define ARROW_UP   0x48
#define ARROW_DOWN 0x50

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

static void irq_callback(_unused registers_t regs) {
    unsigned char c = port_byte_in(0x60);
    if (c == L_SHIFT || c == R_SHIFT) {
        shift_pressed = true;
    } else if (c == L_SHIFT_RELEASE || c == R_SHIFT_RELEASE) {
        shift_pressed = false;
    } else if (c == BACKSPACE) {
        key_buffer_backspace();
    } else if (c == ENTER) {
        key_buffer_return();
    } else if (c == ARROW_UP) {
        shell_handle_up();
    } else if (c == ARROW_DOWN) {
        shell_handle_down();
    }

    if (c > ASCII_MAX) {
//        kprint("keypress ");
//        kprint_uint32(c);
//        kprint(" @ ");
//        kprint_uint32(get_tick());
//        kprint_char('\n');
        return;
    } else if (!shift_pressed && sc_ascii_lower[c]) {
        key_buffer_append(sc_ascii_lower[c]);
    } else if (sc_ascii[c]) {
        key_buffer_append(sc_ascii[c]);
    }
}

void init_keyboard() {
    register_interrupt_handler(IRQ1, &irq_callback);
}