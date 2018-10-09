#include "bmp.h"
#include "kmalloc.h"

#include <stdio.h>
#include <errno.h>

uint8_t *bmp_read_image(const char *filename, bmp_info_header_t *bmp_info_header) {
    FILE *bmp_file;
    bmp_file_header_t header;
    uint8_t *img_data;

    bmp_file = fopen(filename, "r");
    if (ferror(bmp_file))
        return NULL;

    fread(&header, sizeof(bmp_file_header_t), 1, bmp_file);
    if (ferror(bmp_file) || header.type != 0x4D42) {
        fclose(bmp_file);
        return NULL;
    }

    fread(bmp_info_header, sizeof(bmp_info_header_t), 1, bmp_file);
    fseek(bmp_file, header.img_data_offset, SEEK_SET);

    img_data = kmalloc(bmp_info_header->img_data_size);
    if (img_data == NULL) {
        errno = ENOMEM;
        kfree(img_data);
        fclose(bmp_file);
        return NULL;
    }

    fread(img_data, bmp_info_header->img_data_size, 1, bmp_file);
    if (ferror(bmp_file)) {
        fclose(bmp_file);
        return NULL;
    }

    fclose(bmp_file);
    return img_data;
}
