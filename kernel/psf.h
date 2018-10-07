#pragma once

#include <stdint.h>

#define PSF_FONT_MAGIC 0x864ab572

typedef struct psf_font {
    uint32_t magic;           // magic
    uint32_t version;         // zero
    uint32_t header_size;     // offset of bitmaps in file, 32
    uint32_t flags;           // 0 if there's no unicode table
    uint32_t num_glyph;       // number of glyphs
    uint32_t bytes_per_glyph; // size of each glyph
    uint32_t height;          // height in pixels
    uint32_t width;           // width in pixels
} psf_font_t;

void *psf_read_font(const char *filename, psf_font_t *header);