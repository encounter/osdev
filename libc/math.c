#include "math.h"

#include <common.h>

union ldshape {
    long double f;
    struct {
        uint64_t m;
        uint16_t se;
    } i;
};

long double frexpl(long double x, int *e) {
    union ldshape u = {x};
    int ee = u.i.se & 0x7fff;

    if (!ee) {
        if (x) {
            x = frexpl(x * 0x1p120, e);
            *e -= 120;
        } else *e = 0;
        return x;
    } else if (ee == 0x7fff) {
        return x;
    }

    *e = ee - 0x3ffe;
    u.i.se &= 0x8000;
    u.i.se |= 0x3ffe;
    return u.f;
}

int __signbitl(long double x) {
    union ldshape u = {x};
    return u.i.se >> 15;
}

int __fpclassifyl(long double x) {
    union ldshape u = {x};
    int e = u.i.se & 0x7fff;
    int msb = (int) (u.i.m >> 63);
    if (!e && !msb)
        return u.i.m ? FP_SUBNORMAL : FP_ZERO;
    if (e == 0x7fff) {
        /* The x86 variant of 80-bit extended precision only admits
         * one representation of each infinity, with the mantissa msb
         * necessarily set. The version with it clear is invalid/nan.
         * The m68k variant, however, allows either, and tooling uses
         * the version with it clear. */
        if (/*__BYTE_ORDER == __LITTLE_ENDIAN &&*/ !msb)
            return FP_NAN;
        return u.i.m << 1 ? FP_NAN : FP_INFINITE;
    }
    if (!msb)
        return FP_NAN;
    return FP_NORMAL;
}