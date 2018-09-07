#include "drivers/ports.h"
#include "drivers/timer.h"
#include "console.h"
#include "descriptor_tables.h"
#include "isr.h"

#include <stdint.h>
#include <stdnoreturn.h>
#include <string.h>

static void kb_cb(registers_t regs);

_Noreturn void __attribute__((unused)) _start() {
    kprint("Initializing descriptor tables...\n");
    init_descriptor_tables();

    kprint("Testing interrupts...\n");
    __asm__ volatile("int $0x3");
    __asm__ volatile("int $0x4");
    __asm__ volatile("int $0x1f");

    kprint("Initializing timer...\n");
    __asm__ volatile("sti");
//    init_timer(1);
    register_interrupt_handler(IRQ1, &kb_cb);
//    __asm__ volatile("int $33");
//    __asm__ volatile("int $33");

    while (1) {}
}

static void kb_cb(registers_t regs) {
    kprint("keypress @ ");
    kprint_uint32(get_tick());
    kprint_char('\n');
}