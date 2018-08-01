#include "console.h"

#include <stdint.h>
#include <stdnoreturn.h>

_Noreturn void _start() {
    print_str_const("Hello, world!");
    while(1) {}
}