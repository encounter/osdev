#include "psf.h"
#include "kmalloc.h"

#include <stdio.h>
#include <errno.h>

void *psf_read_font(const char *filename, psf_font_t *out_header) {
    FILE *font_file;
    psf_font_t header;
    void *glyphs_data;

    font_file = fopen(filename, "r");
    if (ferror(font_file))
        return NULL;

    fread(&header, sizeof(psf_font_t), 1, font_file);
    if (ferror(font_file) || header.magic != PSF_FONT_MAGIC
        || fseek(font_file, header.header_size, SEEK_SET)) {
        fclose(font_file);
        return NULL;
    }

    uint32_t glyphs_size = header.bytes_per_glyph * header.num_glyph;
    glyphs_data = kmalloc(glyphs_size);
    if (glyphs_data == NULL) {
        errno = ENOMEM;
        fclose(font_file);
        return NULL;
    }

    fread(glyphs_data, glyphs_size, 1, font_file);
    if (ferror(font_file)) {
        fclose(font_file);
        return NULL;
    }
    fclose(font_file);

    *out_header = header;
    return glyphs_data;
}