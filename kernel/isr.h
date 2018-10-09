#pragma once

#include <common.h>

#define IRQ0  32 // Timer
#define IRQ1  33 // Keyboard
#define IRQ2  34 // Cascade for 8259A Slave controller
#define IRQ3  35 // Serial port 2
#define IRQ4  36 // Serial port 1
#define IRQ5  37 // AT systems: Parallel Port 2. PS/2 systems: reserved
#define IRQ6  38 // Diskette drive
#define IRQ7  39 // Parallel Port 1
#define IRQ8  40 // CMOS Real time clock
#define IRQ9  41 // CGA vertical retrace
#define IRQ10 42 // Reserved
#define IRQ11 43 // Reserved
#define IRQ12 44 // AT systems: reserved. PS/2: auxiliary device
#define IRQ13 45 // FPU
#define IRQ14 46 // Hard disk controller
#define IRQ15 47 // Reserved

#define I86_PIC1_REG_COMMAND 0x20 // command register
#define I86_PIC1_REG_STATUS 0x20 // status register
#define I86_PIC1_REG_DATA 0x21 // data register
#define I86_PIC1_REG_IMR 0x21 // interrupt mask register (imr)

#define I86_PIC2_REG_COMMAND 0xA0
#define I86_PIC2_REG_STATUS 0xA0
#define I86_PIC2_REG_DATA 0xA1
#define I86_PIC2_REG_IMR 0xA1

#define		I86_PIC_OCW2_MASK_L1		1		//00000001	//Level 1 interrupt level
#define		I86_PIC_OCW2_MASK_L2		2		//00000010	//Level 2 interrupt level
#define		I86_PIC_OCW2_MASK_L3		4		//00000100	//Level 3 interrupt level
#define		I86_PIC_OCW2_MASK_EOI		0x20		//00100000	//End of Interrupt command
#define		I86_PIC_OCW2_MASK_SL		0x40		//01000000	//Select command
#define		I86_PIC_OCW2_MASK_ROTATE	0x80		//10000000	//Rotation command

typedef struct registers {
    uint32_t cr2, mxcsr;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // Pushed by pusha.
    uint32_t int_no, err_code; // Interrupt number and error code (if applicable)
    uint32_t eip, cs, eflags, useresp, ss; // Pushed by the processor automatically.
} registers_t;

// Enables registration of callbacks for interrupts or IRQs.
// For IRQs, to ease confusion, use the #defines above as the
// first parameter.
typedef void (*isr_t)(registers_t);

void register_interrupt_handler(uint8_t n, isr_t handler);