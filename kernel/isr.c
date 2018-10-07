#include <stdio.h>
#include "isr.h"
#include "console.h"
#include "drivers/ports.h"

isr_t interrupt_handlers[256] = {};

void register_interrupt_handler(uint8_t n, isr_t handler) {
    interrupt_handlers[n] = handler;
}

_unused
void isr_handler(registers_t regs) {
    panic("Received interrupt: %lu (err: %Xh, CR2 %Xh) @ %P\n",
          regs.int_no, regs.err_code, regs.cr2, (uintptr_t) regs.eip);
}

_unused
void irq_handler(registers_t regs) {
    isr_t handler = interrupt_handlers[regs.int_no];
    if (handler) handler(regs);

    // Send an EOI (end of interrupt) signal to the PICs.
    // If this interrupt involved the slave.
    if (regs.int_no >= IRQ8) {
        // Send reset signal to slave.
        port_byte_out(I86_PIC2_REG_COMMAND, I86_PIC_OCW2_MASK_EOI);
    }
    // Send reset signal to master. (As well as slave, if necessary).
    port_byte_out(I86_PIC1_REG_COMMAND, I86_PIC_OCW2_MASK_EOI);
}