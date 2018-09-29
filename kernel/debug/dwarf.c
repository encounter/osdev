#include <malloc.h>
#include <byteswap.h>
#include "dwarf.h"
#include "../elf.h"
#include "../console.h"
#include "../fatfs/ff.h"

#define KERNEL_BASE 0xC0100000

void *dwarf_find_debug_info(FATFS *fs) {
    FRESULT ret;
    FILINFO f_info;
    FIL file;
    void *header_ptr = NULL;
    void *sh_table_ptr = NULL;
    uint32_t read = 0;

    ret = f_stat("kernel.bin", &f_info);
    if (ret != FR_OK || !f_info.fsize) goto fail;

    ret = f_open(&file, "kernel.bin", FA_READ);
    if (ret != FR_OK) goto fail;

    size_t header_size = sizeof(elf_header_t);
    header_ptr = malloc(header_size);
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
    ret = f_read(&file, sh_table_ptr, sh_table_size, &read);
    if (ret != FR_OK || read != sh_table_size) goto fail;

    elf_section_header_t *section = elf_find_section(header, sh_table_ptr, ELF_SHT_STRTAB);
    kprint_uint32((uintptr_t) section);

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
    return NULL;
}