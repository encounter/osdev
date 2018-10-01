#include "stdio.h"
#include "string.h"
#include "errno.h"

#include <limits.h>

int __towrite(FILE *f) {
    f->mode |= f->mode - 1;
    if (f->flags & F_NOWR) {
        f->flags |= F_ERR;
        return EOF;
    }

    /* Clear read buffer (easier than summoning nasal demons) */
    f->rpos = f->rend = 0;

    /* Activate write through the buffer. */
    f->wpos = f->wbase = f->buf;
    f->wend = f->buf + f->buf_size;

    return 0;
}

size_t __fwritex(const char *restrict s, size_t l, FILE *restrict f) {
    size_t i = 0;

    if (!f->wend && __towrite(f)) return 0;

    if (l > f->wend - f->wpos) return f->write(f, s, l);

    if (f->lbf >= 0) {
        /* Match /^(.*\n|)/ */
        for (i = l; i && s[i - 1] != '\n'; i--);
        if (i) {
            if (f->write(f, s, i) < i)
                return i;
            s += i;
            l -= i;
        }
    }

    memcpy(f->wpos, s, l);
    f->wpos += l;
    return l + i;
}

size_t fwrite(const void *restrict src, size_t size, size_t count, FILE *restrict f) {
    size_t k, l = size * count;
    if (!l) return l;
    k = __fwritex(src, l, f);
    return k == l ? count : k / size;
}

int fflush(FILE *f) {
    /* If writing, flush output */
    if (f->wpos != f->wbase) {
        f->write(f, 0, 0);
        if (!f->wpos) {
            return EOF;
        }
    }

    /* If reading, sync position, per POSIX */
    if (f->rpos != f->rend) f->seek(f, f->rpos - f->rend, SEEK_CUR);

    /* Clear read and write modes */
    f->wpos = f->wbase = f->wend = 0;
    f->rpos = f->rend = 0;

    return 0;
}

long ftell(FILE *f) {
    off_t pos = f->seek(f, 0,
                        (f->flags & F_APP) && f->wpos != f->wbase
                        ? SEEK_END : SEEK_CUR);
    if (pos >= 0) {
        /* Adjust for data in buffer. */
        if (f->rend)
            pos += f->rpos - f->rend;
        else if (f->wbase)
            pos += f->wpos - f->wbase;
    }
    if (pos > LONG_MAX) {
        errno = EOVERFLOW;
        return -1;
    }
    return (long) pos;
}
