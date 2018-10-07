#include <stdint.h>
#include <stdio.h>

#define KERNEL_PAGE_OFFSET 0xC0000000 // FIXME make dynamic?

void *load_page_table() {
    void *raw_ptr;
    __asm__ volatile("mov %%cr3, %0" : "=a"(raw_ptr));
    return (uint32_t *) ((uintptr_t) raw_ptr + KERNEL_PAGE_OFFSET);
}

void page_table_set(uintptr_t address, uintptr_t page_addr, uint16_t flags) {
    uint32_t *page_table = load_page_table();
    uint16_t i = (uint16_t) (page_addr >> 22);
    page_table[i] = (address & 0xFFC00000) | flags;
    __asm__ volatile("invlpg %0" : : "m"(i));
}

void *kernel_page_offset(void *ptr) {
    if ((uintptr_t) ptr > KERNEL_PAGE_OFFSET) return ptr;
    return (void *) ((uintptr_t) ptr + KERNEL_PAGE_OFFSET);
}