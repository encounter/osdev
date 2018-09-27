#include "multiboot.h"
#include "console.h"

extern void *malloc_memory_start;
extern void *malloc_memory_end;

struct multiboot_color
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
};

void multiboot_init(uint32_t magic, void *info_ptr) {
    if (magic != MULTIBOOT_MAGIC) {
        panic("multiboot_magic: Invalid magic.\n");
    }

    struct multiboot_info *info = (struct multiboot_info *) info_ptr;
    kprint("flags = ");  kprint_uint32(info->flags); kprint_char('\n');

    if (CHECK_FLAG(info->flags, MULTIBOOT_INFO_MEMORY)) {
        malloc_memory_start = (void *) 0x100000;
        // 1 MiB + (info->mem_upper * 1 KiB)
        malloc_memory_end = malloc_memory_start + (info->mem_upper * 0x400);
        kprint("upper_memory_end = "); kprint_uint32((uintptr_t) malloc_memory_end);
        kprint_char('\n');
    } else {
        panic("multiboot: Memory information required");
    }

    if (CHECK_FLAG(info->flags, MULTIBOOT_INFO_BOOTDEV)) {
        kprint("boot_device = "); kprint_uint32(info->boot_device);
        kprint_char('\n');
    }

    if (CHECK_FLAG(info->flags, MULTIBOOT_INFO_CMDLINE)) {
        kprint("cmdline = "); kprint((char *) info->cmdline);
        kprint_char('\n');
    }

    if (CHECK_FLAG(info->flags, MULTIBOOT_INFO_MODS)) {
        struct multiboot_mod_list *mod;
        int i;

        kprint("mods_count = "); kprint_uint32(info->mods_count);
        kprint(", mods_addr = "); kprint_uint32(info->mods_addr);
        kprint_char('\n');
        for (i = 0, mod = (struct multiboot_mod_list *) info->mods_addr;
             i < info->mods_count;
             i++, mod++) {
            kprint("  mod_start = "); kprint_uint32(mod->mod_start);
            kprint(", mod_end = "); kprint_uint32(mod->mod_end);
            kprint(", cmdline = "); kprint((char *) mod->cmdline);
            kprint_char('\n');
        }
    }

    /* Bits 4 and 5 are mutually exclusive! */
    if (CHECK_FLAG(info->flags, MULTIBOOT_INFO_AOUT_SYMS) &&
        CHECK_FLAG(info->flags, MULTIBOOT_INFO_ELF_SHDR)) {
        panic("multiboot_info->flags: Both bits 4 and 5 are set.\n");
    }

    /* Is the symbol table of a.out valid? */
    if (CHECK_FLAG(info->flags, MULTIBOOT_INFO_AOUT_SYMS)) {
        struct multiboot_aout_symbol_table *aout_sym = &info->u.aout_sym;

        kprint("multiboot_aout_symbol_table: tabsize = "); kprint_uint32(aout_sym->tabsize);
        kprint(", strsize = "); kprint_uint32(aout_sym->strsize);
        kprint(", addr = "); kprint_uint32(aout_sym->addr);
        kprint_char('\n');
    }

    /* Is the section header table of ELF valid? */
    if (CHECK_FLAG(info->flags, MULTIBOOT_INFO_ELF_SHDR)) {
        struct multiboot_elf_section_header_table *elf_sec = &info->u.elf_sec;

        kprint("multiboot_elf_sec: num = "); kprint_uint32(elf_sec->num);
        kprint(", size = "); kprint_uint32(elf_sec->size);
        kprint(", addr = "); kprint_uint32(elf_sec->addr);
        kprint(", shndx = "); kprint_uint32(elf_sec->shndx);
        kprint_char('\n');
    }

    /* Are mmap_* valid? */
    if (CHECK_FLAG(info->flags, MULTIBOOT_INFO_MEM_MAP)) {
        struct multiboot_mmap_entry *mmap;

        kprint("mmap_addr = "); kprint_uint32(info->mmap_addr);
        kprint(", mmap_length = "); kprint_uint32(info->mmap_length);
        kprint_char('\n');

        struct multiboot_mmap_entry *largest_available_entry = NULL;
        for (mmap = (struct multiboot_mmap_entry *) info->mmap_addr;
             (unsigned long) mmap < info->mmap_addr + info->mmap_length;
             mmap = (struct multiboot_mmap_entry *)
                     ((unsigned long) mmap + mmap->size + sizeof(mmap->size))) {
            if (mmap->type == MULTIBOOT_MEMORY_AVAILABLE &&
                (largest_available_entry == NULL || mmap->len > largest_available_entry->len)) {
                largest_available_entry = mmap;
            }

            kprint("  size = "); kprint_uint32(mmap->size);
            kprint(", base_addr = "); kprint_uint64(mmap->addr);
            kprint(", length = "); kprint_uint64(mmap->len);
            kprint(", type = "); kprint_uint32(mmap->type);
            kprint_char('\n');
        }

        if (largest_available_entry != NULL) {
            malloc_memory_start = (void *) (uint32_t) largest_available_entry->addr;
            malloc_memory_end = malloc_memory_start + largest_available_entry->len;
            kprint("malloc_memory_start = "); kprint_uint32((uintptr_t) malloc_memory_start);
            kprint(", end = "); kprint_uint32((uintptr_t) malloc_memory_end);
            kprint_char('\n');
        }
    }

    /* Draw diagonal blue line. */
    if (CHECK_FLAG (info->flags, MULTIBOOT_INFO_FRAMEBUFFER_INFO)) {
        uint32_t color;
        unsigned i;
        void *fb = (void *) (unsigned long) info->framebuffer_addr;

        kprint("framebuffer addr = "); kprint_uint64(info->framebuffer_addr);
        kprint(", type = "); kprint_uint8(info->framebuffer_type);
        kprint(", bpp = "); kprint_uint8(info->framebuffer_bpp);
        kprint_char('\n');

        switch (info->framebuffer_type) {
            case MULTIBOOT_FRAMEBUFFER_TYPE_INDEXED: {
                unsigned best_distance, distance;
                struct multiboot_color *palette;

                palette = (struct multiboot_color *) info->framebuffer_palette_addr;

                color = 0;
                best_distance = 4 * 256 * 256;

                for (i = 0; i < info->framebuffer_palette_num_colors; i++) {
                    distance = (0xff - palette[i].blue) * (0xff - palette[i].blue)
                               + palette[i].red * palette[i].red
                               + palette[i].green * palette[i].green;
                    if (distance < best_distance) {
                        color = i;
                        best_distance = distance;
                    }
                }
            }
                break;

            case MULTIBOOT_FRAMEBUFFER_TYPE_RGB:
                color = ((1 << info->framebuffer_blue_mask_size) - 1)
                        << info->framebuffer_blue_field_position;
                break;

            case MULTIBOOT_FRAMEBUFFER_TYPE_EGA_TEXT:
                console_set_vga_enabled(true);
                color = '\\' | 0x0100;
                break;

            default:
                color = 0xffffffff;
                break;
        }

        for (i = 0; i < info->framebuffer_width
                    && i < info->framebuffer_height; i++) {
            switch (info->framebuffer_bpp) {
                case 8: {
                    uint8_t *pixel = fb + info->framebuffer_pitch * i + i;
                    *pixel = (uint8_t) color;
                }
                    break;
                case 15:
                case 16: {
                    uint16_t *pixel
                            = fb + info->framebuffer_pitch * i + 2 * i;
                    *pixel = (uint16_t) color;
                }
                    break;
                case 24: {
                    uint32_t *pixel
                            = fb + info->framebuffer_pitch * i + 3 * i;
                    *pixel = (color & 0xffffff) | (*pixel & 0xff000000);
                }
                    break;

                case 32: {
                    uint32_t *pixel
                            = fb + info->framebuffer_pitch * i + 4 * i;
                    *pixel = color;
                }
                    break;
            }
        }
    } else {
        console_set_vga_enabled(true); // FIXME
    }
}