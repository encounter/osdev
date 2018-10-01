#pragma once

#define ENOENT           2 // No such file or directory
#define EIO              4 // I/O Error
#define ENOMEM          12 // Out of memory
#define EACCES          13 // Permission denied
#define ENODEV          19 // No such device
#define EINVAL          22 // Invalid argument
#define EMFILE          24 // Too many open files
#define ENOSPC          28 // No space left on device
#define EROFS           30 // Read-only file system
#define ENAMETOOLONG    36 // File name too long
#define ETIME           62 // Timer expired
#define EOVERFLOW       75 // Value too large for defined data type
#define EBADFD          77 // File descriptor in bad state
#define EILSEQ          84 // Illegal byte sequence

extern int errno; // FIXME for threading

char *strerror(int errno);
