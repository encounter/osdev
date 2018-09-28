#include "console.h"
#include "descriptor_tables.h"
#include "drivers/timer.h"
#include "shell.h"
#include "drivers/keyboard.h"
#include "multiboot.h"
#include "drivers/serial.h"
#include "drivers/pci.h"
#include "drivers/ata.h"

#include <common.h>

// #define KDEBUG

extern bool vc_vector_run_tests();

_noreturn _unused
void kernel_main(uint32_t multiboot_magic, void *multiboot_info) {
    serial_init();
    console_set_serial_enabled(true);

    multiboot_init(multiboot_magic, multiboot_info);
    pci_init();
    ata_init();

    clear_screen();
    vc_vector_run_tests();

#ifdef KDEBUG
    kprint("Initializing timer...\n");
    init_timer(1);
#endif

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

    while (1) {
        __asm__ volatile("hlt");
        shell_read();
        key_buffer_print();
    }
}