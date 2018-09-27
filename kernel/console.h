#pragma once

#include <common.h>

bool console_vga_enabled();
void console_set_vga_enabled(bool enabled);

bool console_serial_enabled();
void console_set_serial_enabled(bool enabled);

void clear_screen();

void kprint(const char *message);
void kprint_char(char c);
void kprint_uint64(uint64_t val);
void kprint_uint32(uint32_t val);
void kprint_uint16(uint16_t val);
void kprint_uint8(uint8_t val);
void kprint_backspace();

_noreturn
void panic(char *str);