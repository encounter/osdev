#include "ports.h"

unsigned char port_byte_in(uint16_t port) {
    uint8_t result;
    __asm__ volatile("in %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

void port_byte_out(uint16_t port, uint8_t data) {
    __asm__ volatile("out %0, %1" : : "a"(data), "Nd"(port));
}

uint16_t port_word_in(uint16_t port) {
    uint16_t result;
    __asm__ volatile("in %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

void port_word_out(uint16_t port, uint16_t data) {
    __asm__("out %0, %1" : : "a"(data), "Nd"(port));
}

uint32_t port_long_in(uint16_t port) {
    uint32_t result;
    __asm__ volatile("in %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

void port_long_out(uint16_t port, uint32_t data) {
    __asm__ volatile("out %0, %1" : : "a"(data), "Nd"(port));
}