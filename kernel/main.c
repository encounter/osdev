#include "drivers/ports.h"
#include "drivers/timer.h"
#include "console.h"
#include "descriptor_tables.h"

#include <stdint.h>
#include <stdnoreturn.h>
#include <string.h>

_Noreturn void __attribute__((unused)) _start() {
    kprint("Initializing descriptor tables...\n");
    init_descriptor_tables();

    kprint("Testing interrupts...\n");
    __asm__ volatile("int $0x3");
    __asm__ volatile("int $0x4");
    __asm__ volatile("int $0x1f");

    kprint("Initializing timer...\n");
    __asm__ volatile("sti");
    init_timer(1);

    while (1) {}
}