#include "../fatfs/ff.h"
#include "../console.h"

#include <malloc.h>

bool fatfs_test() {
    FATFS fs;
    FRESULT ret;
    FILINFO info;
    FIL file;
    char *buff = NULL;
    uint32_t read = 0;

    kprint("Mounting drive 0... ");
    ret = f_mount(&fs, "", 1);
    if (ret != FR_OK) goto fail;
    kprint("OK\n");

    ret = f_stat("README", &info);
    if (ret != FR_OK || !info.fsize) goto fail;

    kprint("Opening README with size ");
    kprint_uint64(info.fsize);
    kprint("... ");
    ret = f_open(&file, "README", FA_READ);
    if (ret != FR_OK) goto fail;
    kprint("OK\n");

    buff = malloc((size_t) info.fsize + 1);
    if (buff == NULL) goto fail;

    ret = f_read(&file, buff, (uint32_t) info.fsize, &read);
    if (ret != FR_OK || read != info.fsize) goto fail;

    kprint("File read successfully.\n\n");
//    buff[read] = 0;
//    kprint_char('\n');
//    kprint(buff);
//    kprint_char('\n');

    goto end;

    fail:
    kprint("fatfs fail ");
    kprint_uint8(ret);
    kprint_char('\n');

    end:
    free(buff);
    f_close(&file);
    f_unmount("");
    return ret == FR_OK;
}