#include <stdint.h>
#include <string.h>

int _start(int argc, char **argv) {
    return 5; // ((uintptr_t) strlen) + 256;
}