#include "elf.h"
#include "console.h"

#include <byteswap.h>

#define ELF_DEBUG

elf_header_t *read_elf_header(void *ptr) {
    elf_header_t *header = (elf_header_t *) ptr;
    if (header->magic == ELF_HEADER_MAGIC_LE) {
        kprint("Got success magic ");
        kprint_uint32(header->magic);
        kprint_char('\n');
        if (header->arch_bits != ELF_ARCH_BITS_32) {
#ifdef ELF_DEBUG
            kprint("read_elf_header: incorrect arch_bits = ");
            kprint_uint8(header->arch_bits);
            kprint_char('\n');
#endif
            return NULL;
        }
        if (header->endianness != ELF_ENDIANNESS_LITTLE) {
#ifdef ELF_DEBUG
            kprint("read_elf_header: incorrect endianness = ");
            kprint_uint8(header->endianness);
            kprint_char('\n');
#endif
            return NULL;
        }
        if (header->machine_type != ELF_IS_X86) {
#ifdef ELF_DEBUG
            kprint("read_elf_header: incorrect instruction_set = ");
            kprint_uint16(header->machine_type);
            kprint_char('\n');
#endif
            return NULL;
        }
        if (header->header_size > sizeof(elf_header_t)) {
#ifdef ELF_DEBUG
            kprint("read_elf_header: incorrect header_size = ");
            kprint_uint16(header->header_size);
            kprint_char('\n');
#endif
            return NULL;
        }
        if (header->section_header_entry_size > sizeof(elf_section_header_t)) {
#ifdef ELF_DEBUG
            kprint("read_elf_header: incorrect section header entry size = ");
            kprint_uint16(header->section_header_entry_size);
            kprint(" != ");
            kprint_uint16((uint16_t) sizeof(elf_section_header_t));
            kprint_char('\n');
#endif
            return NULL;
        }
        return header;
    } else {
        kprint("Got bad magic ");
        kprint_uint32(header->magic);
        kprint(" != ");
        kprint_uint32(ELF_HEADER_MAGIC_LE);
        kprint_char('\n');
        return NULL;
    }
}

elf_section_header_t *elf_find_section(elf_header_t *header,
                                       elf_section_header_t *sht_start,
                                       elf_section_header_type_t type) {
    uint16_t num_entries = header->section_header_num_entries;
    kprint("starting scan @ "); kprint_uint32((uintptr_t) sht_start); kprint_char('\n');
    for (uint16_t i = 0; i < num_entries; ++i) {
        elf_section_header_t *section_header = (void *) sht_start + header->section_header_entry_size * i;
        kprint("scanning sect @ "); kprint_uint32((uintptr_t) section_header);
        kprint(", type "); kprint_uint32(section_header->type); kprint_char('\n');
        if (section_header->type == type) return section_header;
    }
    return NULL;
}