#include "descriptor_tables.h"
#include "drivers/ports.h"
#include "isr.h"

extern void gdt_flush(gdt_ptr_t *gdt_ptr);

static void init_gdt();

static void gdt_set_gate(int32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran);

extern void idt_flush(idt_ptr_t *idt_ptr);

static void init_idt();

static void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags);

gdt_entry_t gdt_entries[5] = {};
gdt_ptr_t gdt_ptr;
idt_entry_t idt_entries[256] = {};
idt_ptr_t idt_ptr;

extern isr_t interrupt_handlers[];

// Initialisation routine - zeroes all the interrupt service routines,
// initialises the GDT and IDT.
_unused
void init_descriptor_tables() {
    init_gdt();
    init_idt();
}

static void init_gdt() {
    gdt_ptr.limit = sizeof(gdt_entries) - 1;
    gdt_ptr.base = (uint32_t) &gdt_entries;

    gdt_set_gate(0, 0, 0, 0, 0);                            // Null segment
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0b10011010, 0b11001111); // Code segment
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0b10010010, 0b11001111); // Data segment
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0b11111010, 0b11001111); // User mode code segment
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0b11110010, 0b11001111); // User mode data segment

    gdt_flush(&gdt_ptr);
}

// Set the value of one GDT entry.
static void gdt_set_gate(int32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt_entries[num].base_low = (uint16_t) (base & 0xFFFF);
    gdt_entries[num].base_middle = (uint8_t) (base >> 16 & 0xFF);
    gdt_entries[num].base_high = (uint8_t) (base >> 24 & 0xFF);

    gdt_entries[num].limit_low = (uint16_t) (limit & 0xFFFF);
    gdt_entries[num].granularity = (uint8_t) (limit >> 16 & 0x0F);

    gdt_entries[num].granularity |= gran & 0xF0;
    gdt_entries[num].access = access;
}

static void init_idt() {
    idt_ptr.limit = sizeof(idt_entries) - 1;
    idt_ptr.base = (uint32_t) &idt_entries;

    // Remap the irq table.
    port_byte_out(0x20, 0x11);
    port_byte_out(0xA0, 0x11);
    port_byte_out(0x21, 0x20);
    port_byte_out(0xA1, 0x28);
    port_byte_out(0x21, 0x04);
    port_byte_out(0xA1, 0x02);
    port_byte_out(0x21, 0x01);
    port_byte_out(0xA1, 0x01);
    port_byte_out(0x21, 0x0);
    port_byte_out(0xA1, 0x0);

    idt_set_gate(0, (uint32_t) isr0, 0x08, 0x8E);
    idt_set_gate(1, (uint32_t) isr1, 0x08, 0x8E);
    idt_set_gate(2, (uint32_t) isr2, 0x08, 0x8E);
    idt_set_gate(3, (uint32_t) isr3, 0x08, 0x8E);
    idt_set_gate(4, (uint32_t) isr4, 0x08, 0x8E);
    idt_set_gate(5, (uint32_t) isr5, 0x08, 0x8E);
    idt_set_gate(6, (uint32_t) isr6, 0x08, 0x8E);
    idt_set_gate(7, (uint32_t) isr7, 0x08, 0x8E);
    idt_set_gate(8, (uint32_t) isr8, 0x08, 0x8E);
    idt_set_gate(9, (uint32_t) isr9, 0x08, 0x8E);
    idt_set_gate(10, (uint32_t) isr10, 0x08, 0x8E);
    idt_set_gate(11, (uint32_t) isr11, 0x08, 0x8E);
    idt_set_gate(12, (uint32_t) isr12, 0x08, 0x8E);
    idt_set_gate(13, (uint32_t) isr13, 0x08, 0x8E);
    idt_set_gate(14, (uint32_t) isr14, 0x08, 0x8E);
    idt_set_gate(15, (uint32_t) isr15, 0x08, 0x8E);
    idt_set_gate(16, (uint32_t) isr16, 0x08, 0x8E);
    idt_set_gate(17, (uint32_t) isr17, 0x08, 0x8E);
    idt_set_gate(18, (uint32_t) isr18, 0x08, 0x8E);
    idt_set_gate(19, (uint32_t) isr19, 0x08, 0x8E);
    idt_set_gate(20, (uint32_t) isr20, 0x08, 0x8E);
    idt_set_gate(21, (uint32_t) isr21, 0x08, 0x8E);
    idt_set_gate(22, (uint32_t) isr22, 0x08, 0x8E);
    idt_set_gate(23, (uint32_t) isr23, 0x08, 0x8E);
    idt_set_gate(24, (uint32_t) isr24, 0x08, 0x8E);
    idt_set_gate(25, (uint32_t) isr25, 0x08, 0x8E);
    idt_set_gate(26, (uint32_t) isr26, 0x08, 0x8E);
    idt_set_gate(27, (uint32_t) isr27, 0x08, 0x8E);
    idt_set_gate(28, (uint32_t) isr28, 0x08, 0x8E);
    idt_set_gate(29, (uint32_t) isr29, 0x08, 0x8E);
    idt_set_gate(30, (uint32_t) isr30, 0x08, 0x8E);
    idt_set_gate(31, (uint32_t) isr31, 0x08, 0x8E);

    idt_set_gate(IRQ0, (uint32_t) irq0, 0x08, 0x8E);
    idt_set_gate(IRQ1, (uint32_t) irq1, 0x08, 0x8E);
    idt_set_gate(IRQ2, (uint32_t) irq2, 0x08, 0x8E);
    idt_set_gate(IRQ3, (uint32_t) irq3, 0x08, 0x8E);
    idt_set_gate(IRQ4, (uint32_t) irq4, 0x08, 0x8E);
    idt_set_gate(IRQ5, (uint32_t) irq5, 0x08, 0x8E);
    idt_set_gate(IRQ6, (uint32_t) irq6, 0x08, 0x8E);
    idt_set_gate(IRQ7, (uint32_t) irq7, 0x08, 0x8E);
    idt_set_gate(IRQ8, (uint32_t) irq8, 0x08, 0x8E);
    idt_set_gate(IRQ9, (uint32_t) irq9, 0x08, 0x8E);
    idt_set_gate(IRQ10, (uint32_t) irq10, 0x08, 0x8E);
    idt_set_gate(IRQ11, (uint32_t) irq11, 0x08, 0x8E);
    idt_set_gate(IRQ12, (uint32_t) irq12, 0x08, 0x8E);
    idt_set_gate(IRQ13, (uint32_t) irq13, 0x08, 0x8E);
    idt_set_gate(IRQ14, (uint32_t) irq14, 0x08, 0x8E);
    idt_set_gate(IRQ15, (uint32_t) irq15, 0x08, 0x8E);

    idt_flush(&idt_ptr);
}


static void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt_entries[num].base_lo = (uint16_t) (base & 0xFFFF);
    idt_entries[num].base_hi = (uint16_t) (base >> 16 & 0xFFFF);

    idt_entries[num].sel = sel;
    idt_entries[num].always0 = 0;
    // We must uncomment the OR below when we get to using user-mode.
    // It sets the interrupt gate's privilege level to 3.
    idt_entries[num].flags = flags /* | 0x60 */;
}