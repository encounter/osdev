#include "wchar.h"
#include "stdio.h"
#include "errno.h"

#define MB_CUR_MAX 1 // UTF-8

size_t wcrtomb(char *restrict s, wchar_t wc) {
    if (!s) return 1;
    if ((unsigned) wc < 0x80) {
        *s = (char) wc;
        return 1;
    } else if (MB_CUR_MAX == 1) {
        if (!IS_CODEUNIT(wc)) {
            errno = EILSEQ;
            return (size_t) -1;
        }
        *s = (char) wc;
        return 1;
    } else {
        // utf-8 shit...
    }
}

int wctomb(char *s, wchar_t wc) {
    if (!s) return 0;
    return (int) wcrtomb(s, wc);
}