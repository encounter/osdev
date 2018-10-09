#include "console.h"
#include "descriptor_tables.h"
#include "multiboot.h"
#include "shell.h"
#include "arch/x86/mmu.h"
#include "drivers/ata.h"
#include "drivers/keyboard.h"
#include "drivers/pci.h"
#include "drivers/serial.h"
#include "drivers/timer.h"
#include "drivers/vga.h"
#include "fatfs/ff.h"

#include <common.h>
#include <stdio.h>

#ifdef ENABLE_DWARF
#include "dwarf.h"
#endif

_noreturn _unused
void kernel_main(uint32_t multiboot_magic, void *multiboot_info) {
    serial_init();
    console_set_serial_enabled(true);

    multiboot_init(multiboot_magic, multiboot_info);
    pci_init();
    ata_init();

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

    vga_load_font("assets/default8x16.psfu");
    clear_screen();

    shell_init(fs_mounted);
    __asm__ volatile("sti");

    while (1) {
        __asm__ volatile("hlt");
        __asm__ volatile("cli");
        shell_read();
        key_buffer_print();
        __asm__ volatile("sti");
    }
}
