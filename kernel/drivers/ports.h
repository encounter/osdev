#pragma once

#include <common.h>

#define I86_PIT_REG_COUNTER0 0x40
#define I86_PIT_REG_COUNTER1 0x41
#define I86_PIT_REG_COUNTER2 0x42
#define I86_PIT_REG_COMMAND  0x43

uint8_t port_byte_in(uint16_t port);

void port_byte_out(uint16_t port, uint8_t data);

uint16_t port_word_in(uint16_t port);

void port_word_out(uint16_t port, uint16_t data);

uint32_t port_long_in(uint16_t port);

void port_long_out(uint16_t port, uint32_t data);