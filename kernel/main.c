#include "drivers/ports.h"
#include "drivers/timer.h"
#include "console.h"
#include "descriptor_tables.h"
#include "isr.h"

#include <stdint.h>
#include <stdnoreturn.h>
#include <string.h>
#include <stdbool.h>

static void kb_cb(registers_t regs);

static void mouse_cb(registers_t regs);

void noreturn __attribute__((unused)) _start() {
    kprint("Initializing descriptor tables...\n");
    init_descriptor_tables();

    kprint("Initializing timer...\n");
    init_timer(1);

    kprint("Setting up IRQ handlers...\n");
    register_interrupt_handler(IRQ1, &kb_cb);
    register_interrupt_handler(IRQ12, &mouse_cb);

    kprint("Enabling maskable interrupts...\n");
    __asm__ volatile("sti");

    kprint("Waiting for tick 0x100...");
    while (get_tick() < 0x100) {}
    clear_screen();

    while (1) {}
}

unsigned char kbd_layout[255] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 15
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', // 25
    0, 0, 0, 0, // 29
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', // 38
    0, 0, 0, 0, 0, // 43
    'z', 'x', 'c', 'v', 'b', 'n', 'm', // 50
    0, 0, 0, 0, 0, 0, // 56
    ' ',
};
static bool shift_pressed = false;

static void kb_cb(registers_t regs) {
//    while (!(port_byte_in(0x64) & 1)) {}; // unneeded b/c we're servicing an IRQ
    unsigned char c = port_byte_in(0x60);
    if (c == 0x2A || c == 0x36) {
        shift_pressed = true;
    } else if (c == 0xAA || c == 0xB6) {
        shift_pressed = false;
    } else if (kbd_layout[c]) {
        char kbd_char = kbd_layout[c];
        if (shift_pressed && kbd_char > 0x60 && kbd_char < 0x7B) {
            kbd_char -= 0x20; // roman uppercase
        }
        kprint_char(kbd_char);
    } else {
        kprint("keypress ");
        kprint_uint32(c);
        kprint(" @ ");
        kprint_uint32(get_tick());
        kprint_char('\n');
    }
}

static void mouse_cb(registers_t regs) {
    kprint("mouse @ ");
    kprint_uint32(get_tick());
    kprint_char('\n');
}