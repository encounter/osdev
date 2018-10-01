#include "../fatfs/ff.h"

#include <common.h>
#include <malloc.h>
#include <stdio.h>

bool fatfs_test() {
    FATFS fs;
    FRESULT ret;
    FILINFO info;
    FIL file;
    char *buff = NULL;
    uint32_t read = 0;

    printf("Mounting drive 0... ");
    ret = f_mount(&fs, "", 1);
    if (ret != FR_OK) goto fail;
    printf("OK\n");

    ret = f_stat("README", &info);
    if (ret != FR_OK || !info.fsize) goto fail;

    printf("Opening README with size %llu... \n", info.fsize);
    ret = f_open(&file, "README", FA_READ);
    if (ret != FR_OK) goto fail;
    printf("OK\n");

    buff = malloc((size_t) info.fsize + 1);
    if (buff == NULL) goto fail;

    ret = f_read(&file, buff, (uint32_t) info.fsize, &read);
    if (ret != FR_OK || read != info.fsize) goto fail;

    printf("File read successfully.\n\n");
//    buff[read] = 0;
//    printf("%s\n", (char *) buff);

    goto end;

    fail:
    printf("fatfs fail %d\n", ret);

    end:
    free(buff);
    f_close(&file);
    f_unmount("");
    return ret == FR_OK;
}