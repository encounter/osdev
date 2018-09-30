#include <malloc.h>
#include <byteswap.h>
#include <string.h>
#include <stdio.h>
#include "dwarf.h"
#include "../elf.h"
#include "../console.h"
#include "../fatfs/ff.h"

#define KERNEL_BASE 0xC0100000

void dwarf_find_file(uintptr_t address) {
    FILE *file;
}

void *dwarf_find_debug_info(FATFS *fs) {
    FRESULT ret;
    FILINFO f_info;
    FIL file;
    void *header_ptr = NULL;
    void *sh_table_ptr = NULL;
    void *sh_table_str_ptr = NULL;
    void *debug_line_ptr = NULL;
    uint32_t read = 0;

    ret = f_stat("kernel.bin", &f_info);
    if (ret != FR_OK || !f_info.fsize) goto fail;

    ret = f_open(&file, "kernel.bin", FA_READ);
    if (ret != FR_OK) goto fail;

    size_t header_size = sizeof(elf_header_t);
    header_ptr = malloc(header_size);
    if (header_ptr == NULL) goto fail;

    ret = f_read(&file, header_ptr, header_size, &read);
    if (ret != FR_OK || read != header_size) goto fail;

    elf_header_t *header = read_elf_header(header_ptr);
    if (header == NULL) goto fail;

    ret = f_lseek(&file, header->section_header_offset);
    if (ret != FR_OK) goto fail;

    uint16_t sh_table_size = header->section_header_entry_size *
                             header->section_header_num_entries;
    kprint("sh_table_size = "); kprint_uint16(sh_table_size); kprint_char('\n');
    sh_table_ptr = malloc(sh_table_size);
    if (sh_table_ptr == NULL) goto fail;

    ret = f_read(&file, sh_table_ptr, sh_table_size, &read);
    if (ret != FR_OK || read != sh_table_size) goto fail;

    elf_section_header_t *sh_table_str_section = elf_get_section(header, sh_table_ptr, header->section_header_section_names_idx);
    if (sh_table_str_section == NULL || sh_table_str_section->size > UINT16_MAX) goto fail;

    ret = f_lseek(&file, sh_table_str_section->offset);
    if (ret != FR_OK) goto fail;

    uint16_t sh_table_str_size = (uint16_t) sh_table_str_section->size;
    kprint("sh_table_str_size = "); kprint_uint16(sh_table_str_size); kprint_char('\n');
    sh_table_str_ptr = malloc(sh_table_str_size);
    if (sh_table_str_ptr == NULL) goto fail;

    ret = f_read(&file, sh_table_str_ptr, sh_table_str_size, &read);
    if (ret != FR_OK || read != sh_table_str_size) goto fail;

//    elf_print_sections(header, sh_table_ptr, sh_table_str_ptr);

    elf_section_header_t *debug_line_section = NULL;
    for (uint16_t i = 0; i < header->section_header_num_entries; ++i) {
        elf_section_header_t *section_header = sh_table_ptr + header->section_header_entry_size * i;
        char *name = sh_table_str_ptr + section_header->name;
        if (strcmp(name, ".debug_line") == 0) {
            debug_line_section = section_header;
            break;
        }
    }
    if (debug_line_section == NULL || debug_line_section->size > UINT16_MAX) goto fail;

    ret = f_lseek(&file, debug_line_section->offset);
    if (ret != FR_OK) goto fail;

    uint16_t debug_line_section_size = (uint16_t) debug_line_section->size;
    kprint("debug_line_section_size = "); kprint_uint16(debug_line_section_size); kprint_char('\n');
    debug_line_ptr = malloc(debug_line_section_size);
    if (debug_line_ptr == NULL) goto fail;

    ret = f_read(&file, debug_line_ptr, debug_line_section_size, &read);
    if (ret != FR_OK || read != debug_line_section_size) goto fail;

    dwarf_debug_line_header_t *debug_line_header = debug_line_ptr;
    kprint_uint32(debug_line_header->length);
    kprint_char('\n');

    goto end;

    fail:
    kprint("Failed to read kernel.bin DWARF info: ");
    kprint_uint8(ret);
    kprint(", read: ");
    kprint_uint32(read);
    kprint_char('\n');

    end:
    f_close(&file);
    free(header_ptr);
    free(sh_table_ptr);
    free(sh_table_str_ptr);
    free(debug_line_ptr);
    return NULL;
}