#pragma once

#include <common.h>

typedef uint8_t BYTE;
typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef uint64_t QWORD;
typedef long _LONG;

typedef struct _packed tagBITMAPFILEHEADER {
    WORD bfType;  //specifies the file type
    DWORD bfSize;  //specifies the size in bytes of the bitmap file
    WORD bfReserved1;  //reserved; must be 0
    WORD bfReserved2;  //reserved; must be 0
    DWORD bfOffBits;  //species the offset in bytes from the bitmapfileheader to the bitmap bits
} BITMAPFILEHEADER;

typedef struct _packed tagBITMAPINFOHEADER {
    DWORD biSize;  //specifies the number of bytes required by the struct
    _LONG biWidth;  //specifies width in pixels
    _LONG biHeight;  //species height in pixels
    WORD biPlanes; //specifies the number of color planes, must be 1
    WORD biBitCount; //specifies the number of bit per pixel
    DWORD biCompression;//spcifies the type of compression
    DWORD biSizeImage;  //size of image in bytes
    _LONG biXPelsPerMeter;  //number of pixels per meter in x axis
    _LONG biYPelsPerMeter;  //number of pixels per meter in y axis
    DWORD biClrUsed;  //number of colors used by th ebitmap
    DWORD biClrImportant;  //number of colors that are important
} BITMAPINFOHEADER;

uint8_t *LoadBitmapFile(char *filename, BITMAPINFOHEADER *bitmapInfoHeader);