#pragma once

#define VIDEO_MEM ((volatile uint16_t *) 0xb8000)
#define print_char(pos, c) (VIDEO_MEM[pos] = (0x0F << 8) | c)
#define print_str_const(str) (print_string(str, sizeof(str)))

void print_string(const char *text, int len);
