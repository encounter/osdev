#include <malloc.h>
#include <byteswap.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "dwarf.h"
#include "../elf.h"
#include "../console.h"

#define KERNEL_BASE 0xC0100000

void dwarf_find_file(uintptr_t address) {
    FILE *file;
}

void *dwarf_find_debug_info() {
    void *debug_line_ptr = NULL;
    uint32_t read = 0;

    elf_file_t elf_file = {};
    if (!elf_open(&elf_file, "kernel.bin")) goto fail;
    elf_print_sections(&elf_file);

    // elf_section_header_t *debug_line_section = NULL;
    // for (uint16_t i = 0; i < elf_file.header->section_header_num_entries; ++i) {
    //     elf_section_header_t *section_header = elf_file.sht_start + elf_file.header->section_header_entry_size * i;
    //     char *name = elf_file.sht_str_section + section_header->name;
    //     if (strcmp(name, ".debug_line") == 0) {
    //         debug_line_section = section_header;
    //         break;
    //     }
    // }
    // if (debug_line_section == NULL || debug_line_section->size > UINT16_MAX) goto fail;

//    ret = f_lseek(&file, debug_line_section->offset);
//    if (ret != FR_OK) goto fail;
//
//    uint16_t debug_line_section_size = (uint16_t) debug_line_section->size;
//    kprint("debug_line_section_size = "); kprint_uint16(debug_line_section_size); kprint_char('\n');
//    debug_line_ptr = malloc(debug_line_section_size);
//    if (debug_line_ptr == NULL) goto fail;
//
//    ret = f_read(&file, debug_line_ptr, debug_line_section_size, &read);
//    if (ret != FR_OK || read != debug_line_section_size) goto fail;
//
//    dwarf_debug_line_header_t *debug_line_header = debug_line_ptr;
//    kprint_uint32(debug_line_header->length);
//    kprint_char('\n');

    goto end;

    fail:
    printf("Failed to read DWARF info: %d\n", errno);

    end:
    elf_close(&elf_file);
    free(debug_line_ptr);
    return NULL;
}