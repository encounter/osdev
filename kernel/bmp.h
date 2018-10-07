#pragma once

#include <common.h>

typedef struct _packed bmp_file_header {
    uint16_t type;  //specifies the file type
    uint32_t size;  //specifies the size in bytes of the bitmap file
    uint32_t reserved;
    uint32_t img_data_offset;  //species the offset in bytes from the bitmapfileheader to the bitmap bits
} bmp_file_header_t;

typedef struct _packed bmp_info_header {
    uint32_t header_size;  //specifies the number of bytes required by the struct
    uint32_t width;  //specifies width in pixels
    uint32_t height;  //species height in pixels
    uint16_t planes; //specifies the number of color planes, must be 1
    uint16_t bit_count; //specifies the number of bit per pixel
    uint32_t compression;//spcifies the type of compression
    uint32_t img_data_size;  //size of image in bytes
    uint32_t x_per_meter;  //number of pixels per meter in x axis
    uint32_t y_per_meter;  //number of pixels per meter in y axis
    uint32_t num_colors_used;  //number of colors used by th ebitmap
    uint32_t num_colors_important;  //number of colors that are important
} bmp_info_header_t;

uint8_t *bmp_read_image(const char *filename, bmp_info_header_t *bmp_info_header);