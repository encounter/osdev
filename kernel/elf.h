#pragma once

#include <common.h>
#include <stdio.h>

#define ELF_HEADER_MAGIC_LE ((uint32_t) 0x7F | 'E' << 8 | 'L' << 16 | 'F' << 24)

enum elf_header_arch_bits {
    ELF_ARCH_BITS_32 = 1,
    ELF_ARCH_BITS_64 = 2
};
typedef enum elf_header_arch_bits elf_header_arch_bits_t;

static_assert(sizeof(elf_header_arch_bits_t) == sizeof(uint8_t),
              "elf_header_arch_bits too large");

enum elf_header_endianness {
    ELF_ENDIANNESS_LITTLE = 1,
    ELF_ENDIANNESS_BIG = 2
};
typedef enum elf_header_endianness elf_header_endianness_t;

static_assert(sizeof(elf_header_endianness_t) == sizeof(uint8_t),
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

static_assert(sizeof(elf_machine_type_t) == sizeof(uint16_t),
              "elf_machine_type incorrect size");

enum elf_obj_type {
    ELF_ET_NONE = 0, // No file type
    ELF_ET_REL = 1, // Relocatable file
    ELF_ET_EXEC = 2, // Executable file
    ELF_ET_DYN = 3, // Shared object file (& PIE executables)
    ELF_ET_CORE = 4, // Core file
    ELF_ET_LOPROC = 0xFF00,
    ELF_ET_HIPROC = 0xFFFF
};
typedef enum elf_obj_type elf_obj_type_t;

static_assert(sizeof(elf_obj_type_t) == sizeof(uint16_t),
              "elf_obj_type incorrect size");

struct _packed elf_header {
    uint32_t magic;
    elf_header_arch_bits_t arch_bits;
    elf_header_endianness_t endianness;
    uint8_t elf_version;
    uint8_t __padding[9];
    elf_obj_type_t obj_type;
    elf_machine_type_t machine_type;
    uint32_t version; // ?????
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

static_assert(sizeof(elf_header_t) == 52,
              "elf_header incorrect size");

enum elf_section_header_type {
    ELF_SHT_NULL = 0, // Unused
    ELF_SHT_PROGBITS = 1, // Program bits
    ELF_SHT_SYMTAB = 2, // Symbol table
    ELF_SHT_STRTAB = 3, // String table
    ELF_SHT_RELA = 4, // Relocation entries w/ explicit addends
    ELF_SHT_HASH = 5, // Symbol hash table
    ELF_SHT_DYNAMIC = 6, // Dynamic linking information
    ELF_SHT_NOTE = 7, // Auxiliary information
    ELF_SHT_NOBITS = 8, // Program bits with zero size in file
    ELF_SHT_REL = 9, // Relocation entries w/o explicit addends
    ELF_SHT_SHLIB = 10, // Reserved
    ELF_SHT_DYNSYM = 11, // Dynamic symbol table
    ELF_SHT_LOPROC = 0x70000000,
    ELF_SHT_HIPROC = 0x7FFFFFFF,
    ELF_SHT_LOUSER = 0x80000000,
    ELF_SHT_HIUSER = 0xFFFFFFFF,
};
typedef enum elf_section_header_type elf_section_header_type_t;

static_assert(sizeof(elf_section_header_type_t) == sizeof(uint32_t),
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

//static_assert(sizeof(elf_section_header_t) == 32,
//              "elf_section_header incorrect size");

enum elf_program_header_type {
    ELF_PT_NULL = 0, // Unused
    ELF_PT_LOAD = 1, // Loadable segment
    ELF_PT_DYNAMIC = 2, // Dynamic linking information
    ELF_PT_INTERP = 3, // Interpreter
    ELF_PT_NOTE = 4, // Auxiliary information
    ELF_PT_SHLIB = 5, // Reserved
    ELF_PT_PHDR = 6, // Program header table
    ELF_PT_LOPROC = 0x70000000,
    ELF_PT_HIPROC = 0x7FFFFFFF
};
typedef enum elf_program_header_type elf_program_header_type_t;

struct _packed elf_program_header {
    elf_program_header_type_t type;
    uint32_t offset; // Offset from start of file
    uint32_t vaddr; // Virtual address
    uint32_t paddr; // Physical address (ignored?)
    uint32_t file_size; // Size in file
    uint32_t memory_size; // Size in memory
    uint32_t flags; // ?
    uint32_t align; // Memory alignment
};
typedef struct elf_program_header elf_program_header_t;

struct elf_file {
    FILE *fd;
    elf_header_t *header;
    elf_section_header_t *sht_start;
    char *sht_str_section;
};
typedef struct elf_file elf_file_t;

// --- Public

elf_section_header_t *elf_find_section(elf_file_t *file, const char *name);

elf_section_header_t *elf_get_section(elf_file_t *file, uint16_t index);

void *elf_read_section(elf_file_t *file, elf_section_header_t *section_header);

void elf_print_sections(elf_file_t *file);

elf_file_t *elf_open(const char *filename);

void elf_close(elf_file_t *file);