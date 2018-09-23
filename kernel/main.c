#include "console.h"
#include "descriptor_tables.h"
#include "drivers/keyboard.h"
#include "drivers/ports.h"
#include "drivers/timer.h"
#include "drivers/acpi.h"

#include <stdint.h>
#include <stdnoreturn.h>
#include <string.h>
#include <stdbool.h>
#include <malloc.h>

// #define KDEBUG

static void shell_callback(char *input);

void noreturn __attribute__((unused)) _start() {
#ifdef KDEBUG
    kprint("Initializing descriptor tables...\n");
#endif
    init_descriptor_tables();

#ifdef KDEBUG
    kprint("Initializing timer...\n");
#endif
    init_timer(1);

#ifdef KDEBUG
    kprint("Setting up IRQ handlers...\n");
#endif
    init_keyboard(shell_callback);

#ifdef KDEBUG
    kprint("Enabling maskable interrupts...\n");
#endif
    __asm__ volatile("sti");

#ifdef KDEBUG
    kprint("Waiting for tick 0x100...");
    while (get_tick() < 0x100) {}
    clear_screen();
#endif

    kprint("# ");

    while (1) {}
}

static void shell_callback(char *input) {
    kprint_char('\n');
    unsigned char ret = 1;
    if (strcmp(input, "exit") == 0 ||
               strcmp(input, "poweroff") == 0) {
        port_byte_out(0xf4, 0x00);
        ret = 0;
    } else if (strcmp(input, "reboot") == 0) {
        reboot();
    } else if (strcmp(input, "clear") == 0) {
        clear_screen();
        kprint("# ");
        return;
    }
    kprint_uint32(ret);
    kprint(" # ");
}