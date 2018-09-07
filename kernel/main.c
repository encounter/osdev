#include "drivers/ports.h"
#include "console.h"

#include <stdint.h>
#include <stdnoreturn.h>
#include <string.h>

char *my_text = "Yay! memcpy & strlen.\n";
char *my_tex2 = "                      ";

_Noreturn void _start() {
    kprint("Hello, world!\nHere's my text.\n\nAnd more...\nAnd more...\n\n");
    kprint("Printing some more text now! It should show up in the same place. It should also wrap the screen when the message becomes too long. Yes? No? Maybe?\n");
    for (uint32_t i = 0; i < UINT32_MAX / 32; i++) {}
    kprint("Hello, world!\nHere's my text.\n\nAnd more...\nAnd more...\n\n");
    kprint("Printing some more text now! It should show up in the same place. It should also wrap the screen when the message becomes too long. Yes? No? Maybe?\n");
    for (uint32_t i = 0; i < UINT32_MAX / 32; i++) {}
    kprint("Hello, world!\nHere's my text.\n\nAnd more...\nAnd more...\n\n");
    kprint("Printing some more text now! It should show up in the same place. It should also wrap the screen when the message becomes too long. Yes? No? Maybe?\n");
    // clear_screen();
    for (uint32_t i = 0; i < UINT32_MAX / 32; i++) {}
    kprint("OK! It all works? Somehow...\n");
    for (uint32_t i = 0; i < UINT32_MAX / 32; i++) {}
    kprint("OK! It all works? Somehow...");
    for (uint32_t i = 0; i < UINT32_MAX / 32; i++) {}
    clear_screen();
    memcpy(my_tex2, my_text, strlen(my_text));
    kprint(my_tex2);
    while(1) {}
}