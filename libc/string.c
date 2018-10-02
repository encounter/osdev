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

__attribute__((no_sanitize("alignment")))
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

char *strdup(const char *str) {
    size_t len = strlen(str) + 1;
    char *new = malloc(len);
    if (new == NULL) return NULL;
    return memcpy(new, str, len);
}

__attribute__((no_sanitize("alignment")))
char *__strchrnul(const char *s, int c) {
    c = (unsigned char) c;
    if (!c) return (char *) s + strlen(s);

#ifdef __GNUC__
    typedef size_t __attribute__((__may_alias__)) word;
    const word *w;
    for (; (uintptr_t) s % _ALIGN; s++)
        if (!*s || *(unsigned char *) s == c) return (char *) s;
    size_t k = _ONES * c;
    for (w = (void *) s; !_HASZERO(*w) && !_HASZERO(*w ^ k); w++);
    s = (void *) w;
#endif
    for (; *s && *(unsigned char *) s != c; s++);
    return (char *) s;
}

char *strchr(const char *s, int c) {
    char *r = __strchrnul(s, c);
    return *(unsigned char *) r == (unsigned char) c ? r : 0;
}

size_t strlcpy(char *d, const char *s, size_t n) {
    char *d0 = d;
    size_t *wd;

    if (!n--) goto finish;
#ifdef __GNUC__
    typedef size_t __attribute__((__may_alias__)) word;
    const word *ws;
    if (((uintptr_t) s & _ALIGN) == ((uintptr_t) d & _ALIGN)) {
        for (; ((uintptr_t) s & _ALIGN) && n && (*d = *s); n--, s++, d++);
        if (n && *s) {
            wd = (void *) d;
            ws = (const void *) s;
            for (; n >= sizeof(size_t) && !_HASZERO(*ws);
                   n -= sizeof(size_t), ws++, wd++)
                *wd = *ws;
            d = (void *) wd;
            s = (const void *) ws;
        }
    }
#endif
    for (; n && (*d = *s); n--, s++, d++);
    *d = 0;
    finish:
    return d - d0 + strlen(s);
}


#define PATH_LOOKAHEAD(p, i, c) ((i < len) && (p[i] == c))

// FIXME insane crazy stupid code but it works, I guess
void path_append(char *dest, const char *path, const char *app, size_t len) {
    size_t i = 0, out_idx = 0, dir_idx = 0;

    // Skip existing path if necessary
    if (app != NULL && app[0] == '/') goto append;

    while (i < len && out_idx < len && path[i] != 0) {
        if (path[i] == '/') {
            if (PATH_LOOKAHEAD(path, i + 1, '.')) {
                if (PATH_LOOKAHEAD(path, i + 2, '.')
                    && (PATH_LOOKAHEAD(path, i + 3, '/')
                        || PATH_LOOKAHEAD(path, i + 3, 0))) {
                    out_idx = dir_idx + 1;
                    i += 3;
                    while (PATH_LOOKAHEAD(path, i, '/')) i++;
                    continue;
                } else if (PATH_LOOKAHEAD(path, i + 2, '/')
                           || PATH_LOOKAHEAD(path, i + 2, 0)) {
                    i += 2;
                    continue;
                }
            } else if (PATH_LOOKAHEAD(path, i + 1, '/')) {
                i++;
                continue;
            }
            dir_idx = out_idx;
        }

        dest[out_idx] = path[i];
        i++;
        out_idx++;
    }

    i = 0;
    if (app == NULL || app[0] == 0) goto close;

    // Check for leading . or ..
    if (app[i] == '.') {
        if (PATH_LOOKAHEAD(app, i + 1, '/')) {
            i += 2;
        } else if (PATH_LOOKAHEAD(app, i + 1, 0)) {
            goto close;
        } else if (PATH_LOOKAHEAD(app, i + 1, '.')
                   && (PATH_LOOKAHEAD(app, i + 2, '/')
                       || PATH_LOOKAHEAD(app, i + 2, 0))) {
            out_idx = dir_idx + 1;
            i += 2;
        }
    }

    if (out_idx && dest[out_idx - 1] != '/' && out_idx < len) dest[dir_idx = out_idx++] = '/'; // Add trailing slash
    while (i < len && app[i] == '/') i++; // Skip leading slash(es)

    append:
    while (i < len && out_idx < len && app[i] != 0) {
        if (app[i] == '/') {
            if (PATH_LOOKAHEAD(app, i + 1, '.')) {
                if (PATH_LOOKAHEAD(app, i + 2, '.')
                    && (PATH_LOOKAHEAD(app, i + 3, '/')
                        || PATH_LOOKAHEAD(app, i + 3, 0))) {
                    out_idx = dir_idx + 1;
                    i += 3;
                    while (PATH_LOOKAHEAD(app, i, '/')) i++;
                    continue;
                } else if (PATH_LOOKAHEAD(app, i + 2, '/')
                           || PATH_LOOKAHEAD(app, i + 2, 0)) {
                    i += 2;
                    continue;
                }
            } else if (PATH_LOOKAHEAD(app, i + 1, '/')) {
                i++;
                continue;
            }
            dir_idx = out_idx;
        }

        dest[out_idx] = app[i];
        i++;
        out_idx++;
    }

    close:
    dest[out_idx] = 0;
}