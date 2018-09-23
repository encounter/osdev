#include "console.h"
#include "descriptor_tables.h"
#include "drivers/keyboard.h"
#include "drivers/ports.h"
#include "drivers/timer.h"

#include <stdint.h>
#include <stdnoreturn.h>
#include <string.h>
#include <stdbool.h>
#include <malloc.h>

//static void mouse_cb(registers_t regs);

void noreturn __attribute__((unused)) _start() {
    kprint("Initializing descriptor tables...\n");
    init_descriptor_tables();

    kprint("Initializing timer...\n");
    init_timer(1);

    kprint("Setting up IRQ handlers...\n");
    init_keyboard();
//    register_interrupt_handler(IRQ12, &mouse_cb);

    kprint("Enabling maskable interrupts...\n");
    __asm__ volatile("sti");

//    kprint("Waiting for tick 0x100...");
//    while (get_tick() < 0x100) {}
//    clear_screen();

    while (1) {}
}

//static void mouse_cb(registers_t regs) {
//    kprint("mouse @ ");
//    kprint_uint32(get_tick());
//    kprint_char('\n');
//}