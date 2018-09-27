#pragma once

#include <common.h>

void serial_init();

bool serial_received();

char serial_read();

bool serial_transmit_empty();

void serial_write(char a);