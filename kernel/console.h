#pragma once

#include <common.h>
#include <stdio.h>

bool console_vga_enabled();
void console_set_vga_enabled(bool enabled);

bool console_serial_enabled();
void console_set_serial_enabled(bool enabled);

void clear_screen();

_noreturn
void panic(char *str, ...);

extern FILE *serial;