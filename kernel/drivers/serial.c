#include "serial.h"
#include "ports.h"

#define COM1_BASE 0x3f8

void serial_init() {
    port_byte_out(COM1_BASE + 1, 0x00);    // Disable all interrupts
    port_byte_out(COM1_BASE + 3, 0x80);    // Enable DLAB (set baud rate divisor)
    port_byte_out(COM1_BASE + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
    port_byte_out(COM1_BASE + 1, 0x00);    //                  (hi byte)
    port_byte_out(COM1_BASE + 3, 0x03);    // 8 bits, no parity, one stop bit
    port_byte_out(COM1_BASE + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
    port_byte_out(COM1_BASE + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}

bool serial_received() {
    return (port_byte_in(COM1_BASE + 5) & 1) != 0;
}

char serial_read() {
    while (serial_received() == 0);
    return port_byte_in(COM1_BASE);
}

bool serial_transmit_empty() {
    return (port_byte_in(COM1_BASE + 5) & 0x20) != 0;
}

void serial_write(char a) {
    if (a == '\n') serial_write('\r');
    while (serial_transmit_empty() == 0);
    port_byte_out(COM1_BASE, (unsigned char) a);
}