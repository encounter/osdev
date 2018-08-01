#include "console.h"

#include <stdint.h>

void print_string(const char *text, int len) {
    for (int i = 0; i < len; ++i) {
        print_char(i, text[i]);
    }
}
