#include <stdint.h>

#define VIDEO_MEM ((volatile uint16_t *) 0xb8000)
#define write_char(pos, c) (VIDEO_MEM[pos] = (0x0F << 8) | c)

__attribute__((noreturn)) void _start() {
    write_char(0, 'H');
    write_char(1, 'e');
    write_char(2, 'l');
    write_char(3, 'l');
    write_char(4, 'o');
    while(1) {}
}