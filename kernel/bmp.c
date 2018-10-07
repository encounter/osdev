#include "bmp.h"
#include <stdio.h>
#include <malloc.h>

uint8_t *LoadBitmapFile(char *filename, BITMAPINFOHEADER *bitmapInfoHeader) {
    FILE *filePtr; //our file pointer
    BITMAPFILEHEADER bitmapFileHeader; //our bitmap file header
    unsigned char *bitmapImage;  //store image data
//    int imageIdx = 0;  //image index counter
//    unsigned char tempRGB;  //our swap variable

    //open filename in read binary mode
    filePtr = fopen(filename, "r");
    if (ferror(filePtr))
        return NULL;

    //read the bitmap file header
    fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, filePtr);

    //verify that this is a bmp file by check bitmap id
    if (bitmapFileHeader.bfType != 0x4D42) {
        fclose(filePtr);
        return NULL;
    }

    //read the bitmap info header
    fread(bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, filePtr);

    //move file point to the begging of bitmap data
    fseek(filePtr, bitmapFileHeader.bfOffBits, SEEK_SET);

    printf("Image size: %Xh, offset: %Xh\n", bitmapInfoHeader->biSizeImage, bitmapFileHeader.bfOffBits);

    //allocate enough memory for the bitmap image data
    bitmapImage = (unsigned char *) malloc(bitmapInfoHeader->biSizeImage);

    //verify memory allocation
    if (!bitmapImage) {
        free(bitmapImage);
        fclose(filePtr);
        return NULL;
    }

    //read in the bitmap image data
    fread(bitmapImage, bitmapInfoHeader->biSizeImage, 1, filePtr);

    //make sure bitmap image data was read
    if (ferror(filePtr)) {
        fclose(filePtr);
        return NULL;
    }

    //swap the r and b values to get RGB (bitmap is BGR)
//    for (imageIdx = 0; imageIdx < bitmapInfoHeader->biSizeImage; imageIdx += 3) {
//        tempRGB = bitmapImage[imageIdx];
//        bitmapImage[imageIdx] = bitmapImage[imageIdx + 2];
//        bitmapImage[imageIdx + 2] = tempRGB;
//    }

    //close file and return bitmap iamge data
    fclose(filePtr);
    return bitmapImage;
}
