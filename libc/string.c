#include "string.h"
#include "stdio.h"

#include <limits.h>
#include <malloc.h>

#define _ALIGN (sizeof(size_t))
#define _ONES ((size_t) - 1 / UCHAR_MAX)
#define _HIGHS (_ONES * (UCHAR_MAX / 2 + 1))
#define _HASZERO(x) (((x) - _ONES) & ~(x) & _HIGHS)
#define _SS (sizeof(size_t))

// One measly byte at a time...
void *memcpy(void *restrict destination, const void *restrict source, size_t num) {
    char *s1 = (char *) destination;
    const char *s2 = (const char *) source;

    while (num--) {
        *s1++ = *s2++;
    }

    return destination;
}

void *memmove(void *destination, const void *source, size_t num) {
    char *dest = (char *) destination;
    const char *src = (const char *) source;
    if (dest <= src) {
        while (num--) *dest++ = *src++;
    } else {
        src += num;
        dest += num;
        while (num--) *--dest = *--src;
    }
    return destination;
}

void *memset(void *const ptr, const int value, size_t num) {
    const unsigned char b = (unsigned char) value;
    unsigned char *p = (unsigned char *) ptr;

    while (num--) {
        *p++ = b;
    }

    return ptr;
}

int memcmp(const void *const s1, const void *const s2, size_t n) {
    const unsigned char *p1 = s1;
    const unsigned char *p2 = s2;

    while (n--) {
        const int r = *p1++ - *p2++;
        if (r) return r;
    }

    return 0;
}

void *memchr(const void *src, int c, size_t n) {
    const unsigned char *s = src;
    c = (unsigned char) c;
#ifdef __GNUC__
    for (; ((uintptr_t) s & _ALIGN) && n && *s != c; s++, n--);
    if (n && *s != c) {
        typedef size_t __attribute__((__may_alias__)) word;
        const word *w;
        size_t k = _ONES * c;
        for (w = (const void *) s; n >= _SS && !_HASZERO(*w ^ k); w++, n -= _SS);
        s = (const void *) w;
    }
#endif
    for (; n && *s != c; s++, n--);
    return n ? (void *) s : 0;
}

size_t strlen(const char *str) {
    const char *a = str;
    const size_t *w;
    for (; (uintptr_t) str % _ALIGN; str++) if (!*str) return str - a;
    for (w = (const void *) str; !_HASZERO(*w); w++);
    for (str = (const void *) w; *str; str++);
    return str - a;
}

size_t strnlen(const char *s, size_t n) {
    const char *p = memchr(s, 0, n);
    return p ? p - s : n;
}

int strcmp(const char *const str1, const char *const str2) {
    const unsigned char *p1 = (unsigned char *) str1;
    const unsigned char *p2 = (unsigned char *) str2;

    while (*p1 != '\0' && *p1 == *p2) {
        p1++, p2++;
    }

    return *p1 - *p2;
}

int strncmp(const char *str1, const char *str2, size_t num) {
    while (*str1 && num && (*str1 == *str2)) {
        ++str1;
        ++str2;
        --num;
    }
    return num == 0 ? 0 : *(unsigned char *) str1 - *(unsigned char *) str2;
}

char *strcpy(char *destination, const char *source) {
    char *rc = destination;
    while ((*destination++ = *source++));
    return rc;
}

char *strncpy(char *restrict s1, const char *restrict s2, size_t n) {
    char *rc = s1;
    while ((n > 0) && (*s1++ = *s2++)) --n;
    while (n-- > 1) *s1++ = '\0';
    return rc;
}

char *strdup(char *str) {
    size_t len = strlen(str) + 1;
    char *new = malloc(len);
    if (new == NULL) return NULL;
    return memcpy(new, str, len);
}
