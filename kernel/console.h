#pragma once

#include <common.h>

#define VIDEO_ADDRESS ((volatile uint16_t *) 0xb8000)
#define MAX_ROWS 25
#define MAX_COLS 80

// Attribute byte for our default colour scheme.
#define WHITE_ON_BLACK 0x0F
#define RED_ON_WHITE 0xF4

// Screen device I/O ports
#define REG_SCREEN_CTRL 0x3D4
#define REG_SCREEN_DATA 0x3D5

// Public kernel API
void clear_screen();

void kprint_at(const char *message, int col, int row);

void kprint(const char *message);

void kprint_char(char c);

void kprint_uint32(uint32_t val);

void kprint_backspace();

_noreturn
void panic(char *str);