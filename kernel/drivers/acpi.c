#include <common.h>

#include "ports.h"

#define KBRD_INTRFC 0x64

/* keyboard interface bits */
#define KBRD_BIT_KDATA 0 /* keyboard data is in buffer (output buffer is empty) (bit 0) */
#define KBRD_BIT_UDATA 1 /* user data is in buffer (command buffer is empty) (bit 1) */

#define KBRD_IO 0x60 /* keyboard IO port */
#define KBRD_RESET 0xFE /* reset CPU command */

#define bit(n) (1<<(n))
#define check_flag(flags, n) ((flags) & bit(n))

// Not actually ACPI, but I'll get there eventually.
_noreturn
void reboot() {
    uint8_t temp;

    __asm__ volatile("cli"); /* disable all interrupts */

    /* Clear all keyboard buffers (output and command buffers) */
    do {
        temp = port_byte_in(KBRD_INTRFC); /* empty user data */
        if (check_flag(temp, KBRD_BIT_KDATA) != 0)
            port_byte_in(KBRD_IO); /* empty keyboard data */
    } while (check_flag(temp, KBRD_BIT_UDATA) != 0);

    port_byte_out(KBRD_INTRFC, KBRD_RESET); /* pulse CPU reset line */
    loop:
    __asm__ volatile("hlt"); /* if that didn't work, halt the CPU */
    goto loop; /* if a NMI is received, halt again */
}