#include <stdio.h>
#include <string.h>
#include "multiboot.h"
#include "console.h"
#include "arch/x86/mmu.h"
#include "drivers/vga.h"

#define KERNEL_OFFSET 0x200000 // FIXME
#define BASE_VIDEO_ADDRESS ((void *) 0xB8000)

#define MULTIBOOT_CHECK_FLAG(flags, flag) (((flags) & (flag)) == (flag))

extern void *malloc_memory_start;
extern void *malloc_memory_end;

void multiboot_init(uint32_t magic, void *info_ptr) {
    if (magic != MULTIBOOT_MAGIC) {
        panic("multiboot_magic: Invalid magic "PRIX32"\n", magic);
    }

    struct multiboot_info *info = (struct multiboot_info *) kernel_page_offset(info_ptr);
    printf("multiboot_info = "PRIXPTR", flags = "PRIx32"\n", info, info->flags);

    // Check for base memory information.
    if (MULTIBOOT_CHECK_FLAG(info->flags, MULTIBOOT_INFO_MEMORY)) {
        malloc_memory_start = kernel_page_offset((void *) 0x100000 + KERNEL_OFFSET);
        // 1 MiB + (info->mem_upper * 1 KiB)
        malloc_memory_end = malloc_memory_start + (info->mem_upper * 0x400) - KERNEL_OFFSET;
        printf("upper_memory_end = "PRIXPTR"\n", malloc_memory_end);
    } else {
        panic("multiboot: Memory information required");
    }

    // Check for boot device.
    if (MULTIBOOT_CHECK_FLAG(info->flags, MULTIBOOT_INFO_BOOTDEV)) {
        printf("boot_device = "PRIx32"h\n", info->boot_device);
    }

    // Check for command line.
    if (MULTIBOOT_CHECK_FLAG(info->flags, MULTIBOOT_INFO_CMDLINE)) {
        char *cmdline = (char *) kernel_page_offset((void *) info->cmdline);
        printf("cmdline size %d = %s\n", strlen(cmdline), cmdline);
    }

    // Check for modules.
    if (MULTIBOOT_CHECK_FLAG(info->flags, MULTIBOOT_INFO_MODS)) {
        struct multiboot_mod_list *mod;
        int i;

        printf("mods_count = "PRIu32", mods_addr = "PRIXUPTR"\n", info->mods_count, info->mods_addr);
        for (i = 0, mod = (struct multiboot_mod_list *) kernel_page_offset((void *) info->mods_addr);
             i < info->mods_count;
             i++, mod++) {
            printf("  mod_start = "PRIXUPTR", mod_end = "PRIXUPTR", cmdline = %s\n",
                   mod->mod_start, mod->mod_end, (char *) kernel_page_offset((void *) mod->cmdline));
        }
    }

    // Check for ELF section header table information.
    if (MULTIBOOT_CHECK_FLAG(info->flags, MULTIBOOT_INFO_ELF_SHDR)) {
        struct multiboot_elf_section_header_table *elf_sec = &info->u.elf_sec;
        printf("multiboot_elf_sec: num = "PRIu32", size = "PRIu32", addr = "PRIXUPTR", shndx = "PRIu32"\n",
               elf_sec->num, elf_sec->size, elf_sec->addr, elf_sec->shndx);
    }

    // Check for memory map information
    if (MULTIBOOT_CHECK_FLAG(info->flags, MULTIBOOT_INFO_MEM_MAP)) {
        printf("mmap_addr = "PRIXUPTR", mmap_length = "PRIx32"\n", info->mmap_addr, info->mmap_length);

        struct multiboot_mmap_entry *mmap;
        struct multiboot_mmap_entry *largest_available_entry = NULL;
        for (mmap = (struct multiboot_mmap_entry *) kernel_page_offset((void *) info->mmap_addr);
             (uintptr_t) mmap < (uintptr_t) kernel_page_offset((void *) info->mmap_addr + info->mmap_length);
             mmap = (struct multiboot_mmap_entry *)
                     ((uintptr_t) mmap + mmap->size + sizeof(mmap->size))) {
            if (mmap->type == MULTIBOOT_MEMORY_AVAILABLE &&
                (largest_available_entry == NULL || mmap->len > largest_available_entry->len)) {
                largest_available_entry = mmap;
            }

            printf("  size = "PRIx32", base_addr = "PRIXUPTR64S", length = "PRIX64S", type = "PRIu32"\n",
                   mmap->size, mmap->addr, mmap->len, mmap->type);
        }

        if (largest_available_entry != NULL) {
            malloc_memory_start = kernel_page_offset((void *) (uintptr_t) largest_available_entry->addr + KERNEL_OFFSET);
            malloc_memory_end = malloc_memory_start + largest_available_entry->len - KERNEL_OFFSET;
            printf("malloc_memory_start = "PRIXPTR", end = "PRIXPTR"\n", malloc_memory_start, malloc_memory_end);
        }
    }

    // Check for VBE info.
    if (MULTIBOOT_CHECK_FLAG(info->flags, MULTIBOOT_INFO_VBE_INFO)) {
        printf("vbe_control_info = "PRIXUPTR", vbe_mode_info = "PRIXUPTR", vbe_mode = %d\n",
               info->vbe_control_info, info->vbe_mode_info, info->vbe_mode);
    }

    // Check for VGA framebuffer information.
    if (MULTIBOOT_CHECK_FLAG(info->flags, MULTIBOOT_INFO_FRAMEBUFFER_INFO)) {
        printf("framebuffer_addr = "PRIXUPTR64", type = %d, bpp = %d\n",
               info->framebuffer_addr, info->framebuffer_type, info->framebuffer_bpp);
        uintptr_t fbaddr = (uintptr_t) info->framebuffer_addr;
        page_table_set(fbaddr, 0x83); // Add framebuffer to page table

        vga_init((void *) fbaddr, info->framebuffer_type, &(framebuffer_info_t) {
            .pitch = info->framebuffer_pitch,
            .width = info->framebuffer_width,
            .height = info->framebuffer_height,
            .bpp = info->framebuffer_bpp,
            .palette_addr = info->framebuffer_palette_addr,
            .palette_num_colors = info->framebuffer_palette_num_colors,
            .red_field_position = info->framebuffer_red_field_position,
            .red_mask_size = info->framebuffer_red_mask_size,
            .green_field_position = info->framebuffer_green_field_position,
            .green_mask_size = info->framebuffer_green_mask_size,
            .blue_field_position = info->framebuffer_blue_field_position,
            .blue_mask_size = info->framebuffer_blue_mask_size,
        });

        switch (info->framebuffer_type) {
            case MULTIBOOT_FRAMEBUFFER_TYPE_EGA_TEXT:
                console_set_vga_enabled(true);
                break;

            case MULTIBOOT_FRAMEBUFFER_TYPE_RGB:
                vga_fill_color(255, 255, 255);
                break;

            default:
                break;
        }
    } else {
        // Assume text console & hardcoded FB address for now
        vga_init(kernel_page_offset(BASE_VIDEO_ADDRESS), 2, 0);
        console_set_vga_enabled(true); // FIXME
    }
}