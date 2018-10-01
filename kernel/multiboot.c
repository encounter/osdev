#include <stdio.h>
#include "multiboot.h"
#include "console.h"

#define PAGE_OFFSET 0xC0000000
#define KERNEL_OFFSET 0x100000 // FIXME

extern void *malloc_memory_start;
extern void *malloc_memory_end;

void multiboot_init(uint32_t magic, void *info_ptr) {
    if (magic != MULTIBOOT_MAGIC) {
        panic("multiboot_magic: Invalid magic "PRIX32"\n", magic);
    }

    struct multiboot_info *info = (struct multiboot_info *) ((uintptr_t) info_ptr + PAGE_OFFSET);
    printf("multiboot_info = "PRIXPTR", flags = "PRIx32"\n", info, info->flags);

    if (CHECK_FLAG(info->flags, MULTIBOOT_INFO_MEMORY)) {
        malloc_memory_start = (void *) 0x100000 + PAGE_OFFSET + KERNEL_OFFSET;
        // 1 MiB + (info->mem_upper * 1 KiB)
        malloc_memory_end = malloc_memory_start + (info->mem_upper * 0x400) - KERNEL_OFFSET;
        printf("upper_memory_end = "PRIXPTR"\n", malloc_memory_end);
    } else {
        panic("multiboot: Memory information required");
    }

    if (CHECK_FLAG(info->flags, MULTIBOOT_INFO_BOOTDEV)) {
        printf("boot_device = "PRIx32"h\n", info->boot_device);
    }

    if (CHECK_FLAG(info->flags, MULTIBOOT_INFO_CMDLINE)) {
        printf("cmdline = %s\n", (char *) (info->cmdline + PAGE_OFFSET));
    }

    if (CHECK_FLAG(info->flags, MULTIBOOT_INFO_MODS)) {
        struct multiboot_mod_list *mod;
        int i;

        printf("mods_count = "PRIu32", mods_addr = "PRIXUPTR"\n", info->mods_count, info->mods_addr);
        for (i = 0, mod = (struct multiboot_mod_list *) (info->mods_addr + PAGE_OFFSET);
             i < info->mods_count;
             i++, mod++) {
            printf("  mod_start = "PRIXUPTR", mod_end = "PRIXUPTR", cmdline = %s\n",
                   mod->mod_start, mod->mod_end, (char *) (mod->cmdline + PAGE_OFFSET));
        }
    }

    /* Is the section header table of ELF valid? */
    if (CHECK_FLAG(info->flags, MULTIBOOT_INFO_ELF_SHDR)) {
        struct multiboot_elf_section_header_table *elf_sec = &info->u.elf_sec;
        printf("multiboot_elf_sec: num = "PRIu32", size = "PRIu32", addr = "PRIXUPTR", shndx = "PRIu32"\n",
               elf_sec->num, elf_sec->size, elf_sec->addr, elf_sec->shndx);
    }

    /* Are mmap_* valid? */
    if (CHECK_FLAG(info->flags, MULTIBOOT_INFO_MEM_MAP)) {
        printf("mmap_addr = "PRIXUPTR", mmap_length = "PRIx32"\n", info->mmap_addr, info->mmap_length);

        struct multiboot_mmap_entry *mmap;
        struct multiboot_mmap_entry *largest_available_entry = NULL;
        for (mmap = (struct multiboot_mmap_entry *) (info->mmap_addr + PAGE_OFFSET);
             (unsigned long) mmap < info->mmap_addr + PAGE_OFFSET + info->mmap_length;
             mmap = (struct multiboot_mmap_entry *)
                     ((unsigned long) mmap + mmap->size + sizeof(mmap->size))) {
            if (mmap->type == MULTIBOOT_MEMORY_AVAILABLE &&
                (largest_available_entry == NULL || mmap->len > largest_available_entry->len)) {
                largest_available_entry = mmap;
            }

            printf("  size = "PRIx32", base_addr = "PRIXUPTR64S", length = "PRIX64S", type = "PRIu32"\n",
                   mmap->size, mmap->addr, mmap->len, mmap->type);
        }

        if (largest_available_entry != NULL) {
            malloc_memory_start = (void *) (uint32_t) largest_available_entry->addr + PAGE_OFFSET + KERNEL_OFFSET;
            malloc_memory_end = malloc_memory_start + largest_available_entry->len - KERNEL_OFFSET;
            printf("malloc_memory_start = "PRIXPTR", end = "PRIXPTR"\n", malloc_memory_start, malloc_memory_end);
        }
    }

    /* Check VGA framebuffer. */
    if (CHECK_FLAG (info->flags, MULTIBOOT_INFO_FRAMEBUFFER_INFO)) {
        printf("framebuffer_addr = "PRIXUPTR64", type = %d, bpp = %d\n",
               info->framebuffer_addr, info->framebuffer_type, info->framebuffer_bpp);

        switch (info->framebuffer_type) {
            case MULTIBOOT_FRAMEBUFFER_TYPE_EGA_TEXT:
                console_set_vga_enabled(true);
                break;

            default:
                break;
        }
    } else {
        console_set_vga_enabled(true); // FIXME
    }
}