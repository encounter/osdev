#pragma once

#define EINVAL          22
#define EOVERFLOW       75
#define EILSEQ          84

extern int errno; // FIXME for threading

char *strerror(int errno);
