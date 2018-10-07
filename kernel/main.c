#include "console.h"
#include "descriptor_tables.h"
#include "drivers/timer.h"
#include "shell.h"
#include "drivers/keyboard.h"
#include "multiboot.h"
#include "drivers/serial.h"
#include "drivers/pci.h"
#include "drivers/ata.h"
#include "fatfs/ff.h"

#ifdef ENABLE_DWARF
#include "dwarf.h"
#include "bmp.h"
#include "drivers/vga.h"
#include "arch/x86/mmu.h"

#endif

#include <common.h>
#include <stdio.h>

// #define KDEBUG

_noreturn _unused
void kernel_main(uint32_t multiboot_magic, void *multiboot_info) {
    serial_init();
    console_set_serial_enabled(true);

    multiboot_init(multiboot_magic, multiboot_info);
    bool vga_enabled = console_vga_enabled();
    console_set_vga_enabled(false);
    pci_init();
    ata_init();

    // uint32_t i = UINT32_MAX / 16;
    // while(i--); // stall

    bool fs_mounted = false;
    printf("Mounting drive 0... ");
    FATFS fs;
    FRESULT ret = f_mount(&fs, "", 1);
    if (ret == FR_OK) {
        printf("OK\n");
        fs_mounted = true;
    } else {
        printf("fail %d\n", ret);
    }

#ifdef ENABLE_DWARF
//    dwarf_find_debug_info();
#endif

    console_set_vga_enabled(vga_enabled);
    clear_screen();

    page_table_set(0x400000, 0xC0400000, 0x83);
    BITMAPINFOHEADER bmpHeader;
    uint8_t *bmp = LoadBitmapFile("assets/test.bmp", &bmpHeader);
    if (bmp == NULL) {
        fprintf(stderr, "Failed to read bitmap");
    } else {
        printf("Displaying image...\n");
        vga_display_image_bgr(0, 0, &bmpHeader, bmp);
    }

#ifdef KDEBUG
    kprint("Initializing timer...\n");
    init_timer(1);
#endif

#ifdef KDEBUG
    kprint("Setting up IRQ handlers...\n");
#endif
    shell_init(fs_mounted);

#ifdef KDEBUG
    kprint("Enabling maskable interrupts...\n");
#endif
    __asm__ volatile("sti");

#ifdef KDEBUG
    kprint("Waiting for tick 0x100...");
    while (get_tick() < 0x100) {}
    clear_screen();
#endif

    while (1) {
        __asm__ volatile("hlt");
        __asm__ volatile("cli");
        shell_read();
        key_buffer_print();
        __asm__ volatile("sti");
    }
}
