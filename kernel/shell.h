#pragma once

#include <common.h>

void shell_init(bool fs_mounted);

void shell_read();

void shell_handle_up();

void shell_handle_down();

void key_buffer_clear();

bool key_buffer_append(char c);

void key_buffer_backspace();

void key_buffer_return();