#include "psf.h"
#include <stdio.h>
#include <malloc.h>
#include <errno.h>

void *psf_read_font(const char *filename, psf_font_t *header) {
    FILE *font_file;
    void *glyphs_data;

    font_file = fopen(filename, "r");
    if (ferror(font_file))
        return NULL;

    fread(header, sizeof(psf_font_t), 1, font_file);
    if (ferror(font_file) || header->magic != PSF_FONT_MAGIC) {
        fclose(font_file);
        return NULL;
    }

    fseek(font_file, header->header_size, SEEK_SET);

    uint32_t glyphs_size = header->bytes_per_glyph * header->num_glyph;
    glyphs_data = malloc(glyphs_size);
    if (glyphs_data == NULL) {
        errno = ENOMEM;
        free(glyphs_data);
        fclose(font_file);
        return NULL;
    }

    fread(glyphs_data, glyphs_size, 1, font_file);
    if (ferror(font_file)) {
        fclose(font_file);
        return NULL;
    }

    fclose(font_file);
    return glyphs_data;
}