#pragma once

#include <common.h>

#define ELF_HEADER_MAGIC_LE ((uint32_t) 0x7F | 'E' << 8 | 'L' << 16 | 'F' << 24)

enum elf_header_arch_bits {
    ELF_ARCH_BITS_32 = 1,
    ELF_ARCH_BITS_64 = 2
};
typedef enum elf_header_arch_bits elf_header_arch_bits_t;

_Static_assert(sizeof(elf_header_arch_bits_t) == sizeof(uint8_t),
               "elf_header_arch_bits too large");

enum elf_header_endianness {
    ELF_ENDIANNESS_LITTLE = 1,
    ELF_ENDIANNESS_BIG = 2
};
typedef enum elf_header_endianness elf_header_endianness_t;

_Static_assert(sizeof(elf_header_endianness_t) == sizeof(uint8_t),
               "elf_header_endianness too large");

enum elf_machine_type {
    ELF_IS_NONE = 0,
    ELF_IS_SPARC = 2,
    ELF_IS_X86 = 3,
    ELF_IS_MIPS = 8,
    ELF_IS_POWERPC = 0x14,
    ELF_IS_ARM = 0x28,
    ELF_IS_SUPERH = 0x2A,
    ELF_IS_IA_64 = 0x32,
    ELF_IS_X86_64 = 0x3E,
    ELF_IS_AARCH64 = 0xB7,

    // uint16_t padding
            __ELF_IS_UNUSED = 0x100
};
typedef enum elf_machine_type elf_machine_type_t;

_Static_assert(sizeof(elf_machine_type_t) == sizeof(uint16_t),
               "elf_machine_type incorrect size");

struct _packed elf_header {
    uint32_t magic;
    elf_header_arch_bits_t arch_bits;
    elf_header_endianness_t endianness;
    uint8_t elf_version;
    uint8_t __padding[9];
    uint16_t elf_type; // 1 = relocatable, 2 = executable, 3 = shared, 4 = core
    elf_machine_type_t machine_type;
    uint32_t program_entry_offset;
    uint32_t program_header_offset;
    uint32_t section_header_offset;
    uint32_t flags; // unused on x86
    uint16_t header_size;
    uint16_t program_header_entry_size;
    uint16_t program_header_num_entries;
    uint16_t section_header_entry_size;
    uint16_t section_header_num_entries;
    uint16_t section_header_section_names_idx;
};
typedef struct elf_header elf_header_t;

_Static_assert(sizeof(elf_header_t) == 48,
               "elf_header incorrect size");

enum elf_section_header_type {
    ELF_SHT_NULL = 0,
    ELF_SHT_PROGBITS = 1,
    ELF_SHT_SYMTAB = 2,
    ELF_SHT_STRTAB = 3,
    ELF_SHT_RELA = 4,
    ELF_SHT_HASH = 5,
    ELF_SHT_DYNAMIC = 6,
    ELF_SHT_NOTE = 7,
    ELF_SHT_NOBITS = 8,
    ELF_SHT_REL = 9,
    ELF_SHT_SHLIB = 10,
    ELF_SHT_DYNSYM = 11,
    ELF_SHT_LOPROC = 0x70000000,
    ELF_SHT_HIPROC = 0x7FFFFFFF,
    ELF_SHT_LOUSER = 0x80000000,
    ELF_SHT_HIUSER = 0xFFFFFFFF
};
typedef enum elf_section_header_type elf_section_header_type_t;

_Static_assert(sizeof(elf_section_header_type_t) == sizeof(uint32_t),
               "elf_section_header_type incorrect size");

struct _packed elf_section_header {
    uint32_t name; // name offset in string section
    elf_section_header_type_t type;
    uint32_t flags; // flags
    uint32_t address; // virtual address
    uint32_t offset; // offset from beginning of file
    uint32_t size; // section size in bytes
    uint32_t link; // index link (?)
    uint32_t info; // extra info
    uint32_t address_align; // 0 or 1 = no alignment constraint
    uint32_t entity_size; // 0 = not fixed-size
};
typedef struct elf_section_header elf_section_header_t;

//_Static_assert(sizeof(elf_section_header_t) == 32,
//               "elf_section_header incorrect size");

// --- Public

elf_header_t *read_elf_header(void *ptr);

elf_section_header_t *elf_find_section(elf_header_t *header,
                                       elf_section_header_t *sht_start,
                                       elf_section_header_type_t type);
