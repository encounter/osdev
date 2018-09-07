#include "isr.h"
#include "console.h"

// This gets called from our ASM interrupt handler stub.
void isr_handler(registers_t regs) {
    kprint("recieved interrupt: ");
    kprint_uint32(regs.int_no);
    kprint_char('\n');
}