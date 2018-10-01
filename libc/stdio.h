#pragma once

#include <common.h>
#include <stdarg.h>

#define EOF (-1)

#define F_PERM 1
#define F_NORD 4
#define F_NOWR 8
#define F_EOF 16
#define F_ERR 32
#define F_SVB 64
#define F_APP 128

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

// _IO_FILE definitions
#define BUFSIZ 1024
#define UNGET 8

// Unsigned numbers
#define PRIu32    "%lu"
#define PRIu64    "%llu"

// Numbers as hex (ex. uint32_t)
#define PRIx32     "%lXh"
#define PRIx64     "%llXh"

// Numbers as zero-padded hex (ex. uint32_t)
#define PRIX32     "%08lXh"
#define PRIX64     "%016llXh"
#define PRIX64S    "%08llXh"    // 64-bit w/ 32-bit padding

// Numerical pointers as zero-padded hex (ex. uintptr_t)
#define PRIXUPTR    "0x%08lX"
#define PRIXUPTR64  "0x%016llX"
#define PRIXUPTR64S "0x%08llX"   // 64-bit w/ 32-bit padding

// Pointer value as zero-padded hex (ex. void *)
#define PRIXPTR   "%010P"

typedef struct _IO_FILE FILE;

struct _IO_FILE {
    unsigned flags;
    char *rpos, *rend;
    char *wend, *wpos, *wbase;

    size_t (*read)(FILE *, char *, size_t);

    size_t (*write)(FILE *, const char *, size_t);

    off_t (*seek)(FILE *, off_t, int);

    int (*close)(FILE *);

    char *buf;
    size_t buf_size;
    FILE *prev, *next;
    int fd;
    signed char mode; // 'r' or 'w' I guess
    signed char lbf; // Line buffer (format?)
    int lock;
    void *cookie;
    off_t off;
};

// TODO flesh out
struct stat {
    off_t st_size;
};

extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;

// --- Internal

int __towrite(FILE *f);

size_t __fwritex(const char *restrict s, size_t l, FILE *restrict f);


// --- File

size_t fread(void *restrict dest, size_t size, size_t count, FILE *restrict f);

size_t fwrite(const void *restrict src, size_t size, size_t count, FILE *restrict f);

int fflush(FILE *f);

FILE *fopen(const char *filename, const char *mode);

int fclose(FILE *f);

// int fstat(int fd, struct stat *st); FIXME once fds are implemented
int fstat(const char *filename, struct stat *st);

int fseek(FILE *f, off_t off, int origin);

long ftell(FILE *f);

#define ferror(f) (f == NULL || !!(f->flags & F_ERR))

#define feof(f) (!!(f->flags & F_EOF))

// --- Print

int printf(const char *restrict fmt, ...);

int vprintf(const char *restrict fmt, va_list ap);

int fprintf(FILE *restrict f, const char *restrict fmt, ...);

int sprintf(char *restrict s, const char *restrict fmt, ...);

int snprintf(char *restrict s, size_t n, const char *restrict fmt, ...);

int vfprintf(FILE *restrict f, const char *restrict fmt, va_list ap);

int vsprintf(char *restrict s, const char *restrict fmt, va_list ap);

int vsnprintf(char *restrict s, size_t n, const char *restrict fmt, va_list ap);