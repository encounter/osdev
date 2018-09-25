#pragma once

void init_keyboard(void (*shell_callback)(char *input));

void key_buffer_set(char *input);

void key_buffer_print();