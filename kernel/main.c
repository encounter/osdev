#include "console.h"
#include "descriptor_tables.h"
#include "drivers/timer.h"
#include "shell.h"

#include <stdnoreturn.h>

// #define KDEBUG

void noreturn __attribute__((unused)) kernel_main() {
    clear_screen();

#ifdef KDEBUG
    kprint("Initializing timer...\n");
#endif
    init_timer(1);

#ifdef KDEBUG
    kprint("Setting up IRQ handlers...\n");
#endif
    shell_init();

#ifdef KDEBUG
    kprint("Enabling maskable interrupts...\n");
#endif
    __asm__ volatile("sti");

#ifdef KDEBUG
    kprint("Waiting for tick 0x100...");
    while (get_tick() < 0x100) {}
    clear_screen();
#endif

    while (1) {}
}