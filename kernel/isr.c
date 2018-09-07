#include "isr.h"
#include "console.h"
#include "drivers/ports.h"

isr_t interrupt_handlers[256];

void register_interrupt_handler(uint8_t n, isr_t handler) {
    interrupt_handlers[n] = handler;
}

void __attribute__((unused)) isr_handler(registers_t regs) {
    kprint("Received interrupt: ");
    kprint_uint32(regs.int_no);
    kprint_char('\n');
}

void __attribute__((unused)) irq_handler(registers_t regs) {
    // Send an EOI (end of interrupt) signal to the PICs.
    // If this interrupt involved the slave.
    if (regs.int_no >= 40) {
        // Send reset signal to slave.
        port_byte_out(0xA0, 0x20);
    }
    // Send reset signal to master. (As well as slave, if necessary).
    port_byte_out(0x20, 0x20);

    if (interrupt_handlers[regs.int_no] != 0) {
        isr_t handler = interrupt_handlers[regs.int_no];
        handler(regs);
    }
}