#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <errno.h>
#include <math.h>

#include "fatfs/ff.h"

int __fatfs_to_stderr(FRESULT ret) {
    switch (ret) {
        case FR_OK:
            break;
        case FR_DISK_ERR:
        case FR_MKFS_ABORTED:
            return EIO;
        case FR_INT_ERR:
        case FR_INVALID_PARAMETER:
            return EINVAL;
        case FR_NOT_READY:
        case FR_INVALID_DRIVE:
        case FR_NOT_ENABLED:
            return ENODEV;
        case FR_NO_FILE:
        case FR_NO_PATH:
            return ENOENT;
        case FR_INVALID_NAME:
            return ENAMETOOLONG;
        case FR_DENIED:
            return ENOSPC;
        case FR_EXIST:
        case FR_LOCKED:
            return EACCES;
        case FR_INVALID_OBJECT:
            return EBADFD;
        case FR_WRITE_PROTECTED:
            return EROFS;
        case FR_NO_FILESYSTEM:
            return EILSEQ;
        case FR_TIMEOUT:
            return ETIME;
        case FR_NOT_ENOUGH_CORE:
            return ENOMEM;
        case FR_TOO_MANY_OPEN_FILES:
            return EMFILE;
    }
    return 0;
}

FILE *__fatfs_open(FILE *f, const char *filename, const char *m) {
    FRESULT ret;

    f->cookie = malloc(sizeof(FIL));
    if (f->cookie == NULL) {
        errno = ENOMEM;
        f->flags |= F_ERR;
        return NULL;
    }

    uint8_t mode = FA_READ | FA_WRITE;
    if (f->flags & F_NOWR) mode &= ~FA_WRITE;
    if (f->flags & F_NORD) mode &= ~FA_READ;
    if (f->flags & F_APP) mode |= FA_OPEN_APPEND;
    // TODO FA_CREATE_ALWAYS, FA_OPEN_ALWAYS?

    ret = f_open(f->cookie, filename, mode);
    if (ret != FR_OK) {
        errno = __fatfs_to_stderr(ret);
        f->flags |= F_ERR;
        return NULL;
    }
    
    return f;
}

size_t __fatfs_read(FILE *f, char *buf, size_t len) {
    FRESULT ret;
    size_t read;

    ret = f_read(f->cookie, buf, len, &read);

    if (ret != FR_OK) {
        errno = __fatfs_to_stderr(ret);
        f->flags |= F_ERR;
        return 0;
    } else if (read == 0) {
        f->flags |= F_EOF;
        return 0;
    }
    if (read <= len) return read;
    read -= len;
    // TODO buffer logic?
    //f->rpos = f->buf;
    //f->rend = f->buf + cnt;
    //if (f->buf_size) buf[len-1] = *f->rpos++;
    return len;
}

size_t __fatfs_write(FILE *f, const char *buf, size_t len) {
    FRESULT ret;
    size_t written = 0;
    size_t rem = f->wpos - f->wbase;

    if (rem) {
        ret = f_write(f->cookie, f->wbase, rem, &written);
        if (ret != FR_OK || written != rem) {
            errno = ret == FR_OK ? EIO : __fatfs_to_stderr(ret);
            f->flags |= F_ERR;
            return written;
        }
    }

    if (len) {
        ret = f_write(f->cookie, buf, len, &written);
        if (ret != FR_OK || written != len) {
            errno = ret == FR_OK ? EIO : __fatfs_to_stderr(ret);
            f->flags |= F_ERR;
            return written;
        }
    }

    f->wend = f->buf + f->buf_size;
    f->wpos = f->wbase = f->buf;
    return len;
}

off_t __fatfs_seek(FILE *f, const off_t offset, int origin) {
    FRESULT ret;
    FIL *fp = f->cookie;
    off_t ofs = offset;

    if (origin == SEEK_CUR) ofs += f_tell(fp);
    if (origin == SEEK_END) ofs += f_size(fp);
    ret = f_lseek(fp, (FSIZE_t) ofs);
    if (ret != FR_OK) {
        errno = __fatfs_to_stderr(ret);
        f->flags |= F_ERR;
        return EOF;
    }

    return ofs;
}

int __fatfs_close(FILE *f) {
    FRESULT ret = f_close(f->cookie);
    if (ret != FR_OK) {
        errno = __fatfs_to_stderr(ret);
        f->flags |= F_ERR;
        return EOF;
    }
    free(f->cookie);
    return 0;
}

FILE *fopen(const char *filename, const char *mode) {
    FILE *f;

    /* Check for valid initial mode character */
    if (!strchr("rwa", *mode)) {
        errno = EINVAL;
        return NULL;
    }

    if (!(f = malloc(sizeof(FILE) + UNGET + BUFSIZ))) {
        errno = ENOMEM;
        return NULL;
    }
    memset(f, 0, sizeof(FILE));

    /* Impose mode restrictions */
    if (!strchr(mode, '+')) f->flags = (*mode == 'r') ? F_NOWR : F_NORD;

    /* Set append mode on fd if opened for append */
    if (*mode == 'a') f->flags |= F_APP;

    // f->fd = fd;
    f->buf = (char *) f + sizeof(FILE) + UNGET;
    f->buf_size = BUFSIZ;

    /* Activate line buffered mode for terminals */
    f->lbf = EOF;
    if (!(f->flags & F_NOWR)) f->lbf = '\n';

    f->read = __fatfs_read;
    f->write = __fatfs_write;
    f->seek = __fatfs_seek;
    f->close = __fatfs_close;

    return __fatfs_open(f, filename, mode);
}

static int __toread(FILE *f) {
    f->mode |= f->mode - 1;
    if (f->wpos != f->wbase) f->write(f, 0, 0);
    f->wpos = f->wbase = f->wend = 0;
    if (f->flags & F_NORD) {
        f->flags |= F_ERR;
        return EOF;
    }
    f->rpos = f->rend = f->buf + f->buf_size;
    return (f->flags & F_EOF) ? EOF : 0;
}


size_t fread(void *restrict destv, size_t size, size_t nmemb, FILE *restrict f) {
    char *dest = destv;
    size_t len = size * nmemb, l = len, k;
    if (!size) nmemb = 0;

    f->mode |= f->mode - 1;

    if (f->rpos != f->rend) {
        /* First exhaust the buffer. */
        k = MIN(f->rend - f->rpos, l);
        memcpy(dest, f->rpos, k);
        f->rpos += k;
        dest += k;
        l -= k;
    }

    /* Read the remainder directly */
    for (; l; l -= k, dest += k) {
        k = __toread(f) ? 0 : f->read(f, dest, l);
        if (!k) {
            return (len - l) / size;
        }
    }

    return nmemb;
}

int fclose(FILE *f) {
    int r;
    int perm;

    if (!(perm = f->flags & F_PERM)) {
        if (f->prev) f->prev->next = f->next;
        if (f->next) f->next->prev = f->prev;
    }

    r = fflush(f);
    r |= f->close(f);

    // free(f->getln_buf); FIXME not implemented
    if (!perm) free(f);

    return r;
}

int fstat(const char *filename, struct stat *st) {
    FILINFO info;
    FRESULT ret = f_stat(filename, &info);
    if (ret != FR_OK) {
        errno = __fatfs_to_stderr(ret);
        return EOF;
    }

    st->st_size = info.fsize;
    return 0;
}

int fseek(FILE *f, off_t off, int origin) {
    /* Adjust relative offset for unread data in buffer, if any. */
    if (origin == SEEK_CUR && f->rend) off -= f->rend - f->rpos;

    /* Flush write buffer, and report error on failure. */
    if (f->wpos != f->wbase) {
        f->write(f, 0, 0);
        if (!f->wpos) return -1;
    }

    /* Leave writing mode */
    f->wpos = f->wbase = f->wend = 0;

    /* Perform the underlying seek. */
    if (f->seek(f, off, origin) < 0) return -1;

    /* If seek succeeded, file is seekable and we discard read buffer. */
    f->rpos = f->rend = 0;
    f->flags &= ~F_EOF;

    return 0;
}

