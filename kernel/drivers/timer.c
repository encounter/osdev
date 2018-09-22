#include "timer.h"
#include "../isr.h"
#include "../console.h"
#include "ports.h"

uint32_t tick = 0;

static void timer_callback(registers_t regs) {
    tick++;
//    kprint("Tick: ");
//    kprint_uint32(tick);
//    kprint_char('\n');
}

void init_timer(uint32_t frequency) {
    // Firstly, register our timer callback.
    register_interrupt_handler(IRQ0, &timer_callback);

    // The value we send to the PIT is the value to divide it's input clock
    // (1193180 Hz) by, to get our required frequency. Important to note is
    // that the divisor must be small enough to fit into 16-bits.
    uint16_t divisor = (uint16_t) (1193180 / frequency);

    // Send the command byte.
    port_byte_out(I86_PIT_REG_COMMAND, 0x36);

    // Send the frequency divisor.
    port_byte_out(I86_PIT_REG_COUNTER0, (uint8_t) (divisor & 0xFF));
    port_byte_out(I86_PIT_REG_COUNTER0, (uint8_t) (divisor >> 8 & 0xFF));

    tick = 0;
}

uint32_t get_tick() {
    return tick;
}