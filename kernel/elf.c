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
            return false;
        }
        if (header->endianness != ELF_ENDIANNESS_LITTLE) {
            return false;
        }
        if (header->machine_type != ELF_IS_X86) {
            return false;
        }
        if (header->header_size > sizeof(elf_header_t)) {
            return false;
        }
        if (header->section_header_entry_size > sizeof(elf_section_header_t)) {
            return false;
        }
        return true;
    } else {
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
    if (ferror(file->fd) || !read || !_check_elf_header(buf)) {
        free(buf);
        return false;
    }

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
    if ((buf = malloc(sh_table_size)) == NULL) {
        errno = ENOMEM;
        return false;
    }
    read = fread(buf, sh_table_size, 1, file->fd);
    if (ferror(file->fd) || !read) {
        free(buf);
        return false;
    }

    file->sht_start = buf;
    return true;
}

static bool _elf_read_sht_str_section(elf_file_t *file) {
    if (file->sht_str_section != NULL) return true;
    if (!_elf_read_header(file)) return false;

    elf_header_t *header = file->header;
    elf_section_header_t *section_header = elf_get_section(file, header->section_header_section_names_idx);
    if (section_header == NULL) return NULL;
    file->sht_str_section = elf_read_section(file, section_header);
    return file->sht_str_section != NULL;
}

elf_section_header_t *elf_find_section(elf_file_t *file, const char *name) {
    if (!_elf_read_section_header_table(file)
        || !_elf_read_sht_str_section(file))
        return NULL;

    uint16_t num_entries = file->header->section_header_num_entries;
    for (uint16_t i = 0; i < num_entries; ++i) {
        elf_section_header_t *section_header = (void *) file->sht_start + file->header->section_header_entry_size * i;
        if (strcmp(name, file->sht_str_section + section_header->name) == 0) return section_header;
    }
    return NULL;
}

elf_section_header_t *elf_get_section(elf_file_t *file,
                                      uint16_t index) {
    if (!_elf_read_section_header_table(file))
        return NULL;

    uint16_t num_entries = file->header->section_header_num_entries;
    if (index > num_entries - 1) return NULL;
    return (void *) file->sht_start + file->header->section_header_entry_size * index;
}

void *elf_read_section(elf_file_t *file, elf_section_header_t *section_header) {
    if (!_elf_read_header(file)) return false;

    if (section_header == NULL ||
        section_header->size > UINT16_MAX ||
        fseek(file->fd, section_header->offset, SEEK_SET))
        return NULL;

    void *buf = NULL;
    uint16_t section_size = (uint16_t) section_header->size;
    if ((buf = malloc(section_size)) == NULL) {
        errno = ENOMEM;
        return NULL;
    }
    if (!fread(buf, section_size, 1, file->fd) || ferror(file->fd)) {
        free(buf);
        return NULL;
    }

    return buf;
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

static const char *elf_obj_type(elf_obj_type_t type) {
    switch (type) {
        case ELF_ET_NONE: return "None";
        case ELF_ET_REL: return "Relocatable";
        case ELF_ET_EXEC: return "Executable";
        case ELF_ET_DYN: return "Dynamic";
        case ELF_ET_CORE: return "Core";
        default: return "UNKNOWN";
    }
}

void elf_print_sections(elf_file_t *file) {
    if (!_elf_read_header(file)
        || !_elf_read_section_header_table(file)
        || !_elf_read_sht_str_section(file))
        return;

    elf_header_t *header = file->header;
    printf("ELF type: %s\n", elf_obj_type(header->obj_type));
    printf("ELF entry offset: %Xh\n", header->program_entry_offset);

    uint16_t num_entries = header->section_header_num_entries;
    for (uint16_t i = 0; i < num_entries; ++i) {
        elf_section_header_t *section_header = (void *) file->sht_start + header->section_header_entry_size * i;
        if (section_header->type == ELF_SHT_NULL) continue; // Skip null header
        printf("Section %03d %s: offset "PRIx32", size "PRIx32", VMA "PRIXUPTR", type %s\n",
               i, file->sht_str_section + section_header->name,
               section_header->offset, section_header->size,
               section_header->address,
               elf_section_type(section_header->type));
    }
}

elf_file_t *elf_open(const char *filename) {
    elf_file_t *file = malloc(sizeof(elf_file_t));
    if (file == NULL) {
        errno = ENOMEM;
        return NULL;
    }
    memset(file, 0, sizeof(elf_file_t));

    file->fd = fopen(filename, "r");
    if (ferror(file->fd)
        || !_elf_read_header(file)
        || !_elf_read_section_header_table(file)) {
        elf_close(file);
        return NULL;
    }

    return file;
}

void elf_close(elf_file_t *file) {
    if (file == NULL) return;
    if (file->fd != NULL) fclose(file->fd);
    free(file->header);
    free(file->sht_start);
    free(file->sht_str_section);
    free(file);
}