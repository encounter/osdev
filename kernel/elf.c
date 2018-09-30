#include "elf.h"
#include "console.h"

#include <string.h>
#include <stdio.h>

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
    for (uint16_t i = 0; i < num_entries; ++i) {
        elf_section_header_t *section_header = (void *) sht_start + header->section_header_entry_size * i;
        if (section_header->type == type) return section_header;
    }
    return NULL;
}

elf_section_header_t *elf_get_section(elf_header_t *header,
                                      elf_section_header_t *sht_start,
                                      uint16_t index) {
    uint16_t num_entries = header->section_header_num_entries;
    if (index > num_entries - 1) return NULL;
    return (void *) sht_start + header->section_header_entry_size * index;
}

static const char* elf_section_type(elf_section_header_type_t type) {
    switch (type) {
        case ELF_SHT_NULL: return "SHT_NULL";
        case ELF_SHT_PROGBITS: return "SHT_PROGBITS";
        case ELF_SHT_SYMTAB: return "SHT_SYMTAB";
        case ELF_SHT_STRTAB: return "SHT_STRTAB";
        case ELF_SHT_RELA: return "SHT_RELA";
        case ELF_SHT_HASH: return "SHT_HASH";
        case ELF_SHT_DYNAMIC: return "SHT_DYNAMIC";
        case ELF_SHT_NOTE: return "SHT_NOTE";
        case ELF_SHT_NOBITS: return "SHT_NOBITS";
        case ELF_SHT_REL: return "SHT_REL";
        case ELF_SHT_SHLIB: return "SHT_SHLIB";
        case ELF_SHT_DYNSYM: return "SHT_DYNSYM";
        case ELF_SHT_LOPROC: return "SHT_LOPROC";
        case ELF_SHT_HIPROC: return "SHT_HIPROC";
        case ELF_SHT_LOUSER: return "SHT_LOUSER";
        case ELF_SHT_HIUSER: return "SHT_HIUSER";
        default: return "UNKNOWN";
    }
}

void elf_print_sections(elf_header_t *header, elf_section_header_t *sht_start, void *shstrtab_ptr) {
    uint16_t num_entries = header->section_header_num_entries;
    for (uint16_t i = 0; i < num_entries; ++i) {
        elf_section_header_t *section_header = (void *) sht_start + header->section_header_entry_size * i;
        if (section_header->type == ELF_SHT_NULL) continue; // Skip null header
        printf("Section %d %s: offset "PRIx32", size "PRIx32", type %s\n",
                i, (char *) shstrtab_ptr + section_header->name, section_header->offset,
                section_header->size, elf_section_type(section_header->type));
    }
}