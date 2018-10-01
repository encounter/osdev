#include "elf.h"
#include "console.h"

#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <malloc.h>

// #define ELF_DEBUG

bool _check_elf_header(elf_header_t *header) {
    if (header->magic == ELF_HEADER_MAGIC_LE) {
        if (header->arch_bits != ELF_ARCH_BITS_32) {
#ifdef ELF_DEBUG
            kprint("read_elf_header: incorrect arch_bits = ");
            kprint_uint8(header->arch_bits);
            kprint_char('\n');
#endif
            return false;
        }
        if (header->endianness != ELF_ENDIANNESS_LITTLE) {
#ifdef ELF_DEBUG
            kprint("read_elf_header: incorrect endianness = ");
            kprint_uint8(header->endianness);
            kprint_char('\n');
#endif
            return false;
        }
        if (header->machine_type != ELF_IS_X86) {
#ifdef ELF_DEBUG
            kprint("read_elf_header: incorrect instruction_set = ");
            kprint_uint16(header->machine_type);
            kprint_char('\n');
#endif
            return false;
        }
        if (header->header_size > sizeof(elf_header_t)) {
#ifdef ELF_DEBUG
            kprint("read_elf_header: incorrect header_size = ");
            kprint_uint16(header->header_size);
            kprint_char('\n');
#endif
            return false;
        }
        if (header->section_header_entry_size > sizeof(elf_section_header_t)) {
#ifdef ELF_DEBUG
            kprint("read_elf_header: incorrect section header entry size = ");
            kprint_uint16(header->section_header_entry_size);
            kprint(" != ");
            kprint_uint16((uint16_t) sizeof(elf_section_header_t));
            kprint_char('\n');
#endif
            return false;
        }
        return true;
    } else {
        printf("_check_elf_header: Got bad magic %04lu != %04lu\n", header->magic, ELF_HEADER_MAGIC_LE);
        return false;
    }
}

static bool _elf_read_header(elf_file_t *file) {
    if (file->header != NULL) return true;
    if (fseek(file->fd, 0, SEEK_SET)) return false;

    void *buf = NULL;
    size_t header_size = sizeof(elf_header_t), read = 0;
    if ((buf = malloc(header_size)) == NULL) {
        errno = ENOMEM;
        return false;
    }
    read = fread(buf, header_size, 1, file->fd);
    if (ferror(file->fd) || !read || !_check_elf_header(buf)) return false; // FIXME memory leak

    file->header = buf;
    return true;
}

static bool _elf_read_section_header_table(elf_file_t *file) {
    if (file->sht_start != NULL) return true;
    if (!_elf_read_header(file)) return false;

    size_t read = 0;
    elf_header_t *header = file->header;
    if (fseek(file->fd, header->section_header_offset, SEEK_SET))
        return false;

    void *buf = NULL;
    uint16_t sh_table_size = header->section_header_entry_size *
                             header->section_header_num_entries;
    if ((buf = malloc(sh_table_size)) == NULL){
        errno = ENOMEM;
        return false;
    }
    read = fread(buf, sh_table_size, 1, file->fd);
    if (ferror(file->fd) || !read) return false; // FIXME memory leak

    file->sht_start = buf;
    return true;
}

static bool _elf_read_sht_str_section(elf_file_t *file) {
    if (file->sht_str_section != NULL) return true;
    if (!_elf_read_header(file)) return false;

    size_t read = 0;
    elf_header_t *header = file->header;
    elf_section_header_t *sht_str_header = elf_get_section(file, header->section_header_section_names_idx);
    if (sht_str_header == NULL ||
        sht_str_header->size > UINT16_MAX ||
        fseek(file->fd, sht_str_header->offset, SEEK_SET))
        return false;

    void *buf = NULL;
    uint16_t sht_str_size = (uint16_t) sht_str_header->size;
    if ((buf = malloc(sht_str_size)) == NULL){
        errno = ENOMEM;
        return false;
    }
    read = fread(buf, sht_str_size, 1, file->fd);
    if (ferror(file->fd) || !read) return false; // FIXME memory leak

    file->sht_str_section = buf;
    return true;
}

elf_section_header_t *elf_find_section(elf_file_t *file,
                                       elf_section_header_type_t type) {
    uint16_t num_entries = file->header->section_header_num_entries;
    for (uint16_t i = 0; i < num_entries; ++i) {
        elf_section_header_t *section_header = (void *) file->sht_start + file->header->section_header_entry_size * i;
        if (section_header->type == type) return section_header;
    }
    return NULL;
}

elf_section_header_t *elf_get_section(elf_file_t *file,
                                      uint16_t index) {
    uint16_t num_entries = file->header->section_header_num_entries;
    if (index > num_entries - 1) return NULL;
    return (void *) file->sht_start + file->header->section_header_entry_size * index;
}

static const char *elf_section_type(elf_section_header_type_t type) {
    switch (type) {
        case ELF_SHT_NULL:
            return "SHT_NULL";
        case ELF_SHT_PROGBITS:
            return "SHT_PROGBITS";
        case ELF_SHT_SYMTAB:
            return "SHT_SYMTAB";
        case ELF_SHT_STRTAB:
            return "SHT_STRTAB";
        case ELF_SHT_RELA:
            return "SHT_RELA";
        case ELF_SHT_HASH:
            return "SHT_HASH";
        case ELF_SHT_DYNAMIC:
            return "SHT_DYNAMIC";
        case ELF_SHT_NOTE:
            return "SHT_NOTE";
        case ELF_SHT_NOBITS:
            return "SHT_NOBITS";
        case ELF_SHT_REL:
            return "SHT_REL";
        case ELF_SHT_SHLIB:
            return "SHT_SHLIB";
        case ELF_SHT_DYNSYM:
            return "SHT_DYNSYM";
        case ELF_SHT_LOPROC:
            return "SHT_LOPROC";
        case ELF_SHT_HIPROC:
            return "SHT_HIPROC";
        case ELF_SHT_LOUSER:
            return "SHT_LOUSER";
        case ELF_SHT_HIUSER:
            return "SHT_HIUSER";
        default:
            return "UNKNOWN";
    }
}

void elf_print_sections(elf_file_t *file) {
    if (!_elf_read_header(file)
        || !_elf_read_section_header_table(file)
        || !_elf_read_sht_str_section(file))
        return;

    elf_header_t *header = file->header;
    uint16_t num_entries = header->section_header_num_entries;
    for (uint16_t i = 0; i < num_entries; ++i) {
        elf_section_header_t *section_header = (void *) file->sht_start + header->section_header_entry_size * i;
        if (section_header->type == ELF_SHT_NULL) continue; // Skip null header
        printf("Section %03d %s: offset "PRIx32", size "PRIx32", type %s\n",
               i, file->sht_str_section + section_header->name,
               section_header->offset, section_header->size,
               elf_section_type(section_header->type));
    }
}

bool elf_open(elf_file_t *file, const char *filename) {
    struct stat st;

    if (fstat(filename, &st) || !st.st_size) goto fail;

    file->fd = fopen(filename, "r");
    if (ferror(file->fd))
        goto fail;

    if (!_elf_read_header(file) || !_elf_read_section_header_table(file)) goto fail;

    return true;

    fail:
    printf("Failed to read ELF file: %d\n", errno);
    return false;
}

void elf_close(elf_file_t *file) {
    if (file == NULL) return;
    if (file->fd != NULL) fclose(file->fd);
    free(file->header);
    free(file->sht_start);
    free(file->sht_str_section);
}