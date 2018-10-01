#pragma once

// All 32-bit x86 for now

typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef signed short int int16_t;
typedef unsigned short int uint16_t;
typedef signed int int32_t;
typedef unsigned int uint32_t;
typedef signed long long int int64_t;
typedef unsigned long long int uint64_t;

typedef long long int intmax_t;
typedef unsigned long long int uintmax_t;

typedef signed char int_fast8_t;
typedef unsigned char uint_fast8_t;
typedef int int_fast16_t;
typedef unsigned int uint_fast16_t;
typedef int int_fast32_t;
typedef unsigned int uint_fast32_t;
typedef long long int int_fast64_t;
typedef unsigned long long int uint_fast64_t;

typedef int intptr_t;
typedef unsigned int uintptr_t;

typedef int32_t off_t;

/* Signed.  */
#define INT8_C(c)  c
#define INT16_C(c) c
#define INT32_C(c) c
#define INT64_C(c)    c ## LL

/* Unsigned.  */
#define UINT8_C(c) c
#define UINT16_C(c)    c
#define UINT32_C(c)    c ## U
#define UINT64_C(c)   c ## ULL

/* Maximal type.  */
#define INTMAX_C(c)   c ## LL
#define UINTMAX_C(c)  c ## ULL

/* Minimum of signed integral types.  */
#define INT8_MIN       (-128)
#define INT16_MIN      (-32767-1)
#define INT32_MIN      (-2147483647-1)
#define INT64_MIN      (-INT64_C(9223372036854775807)-1)

/* Maximum of signed integral types.  */
#define INT8_MAX       (127)
#define INT16_MAX      (32767)
#define INT32_MAX      (2147483647)
#define INT64_MAX      (INT64_C(9223372036854775807))

/* Maximum of unsigned integral types.  */
#define UINT8_MAX      (255)
#define UINT16_MAX     (65535)
#define UINT32_MAX     (4294967295U)
#define UINT64_MAX     (UINT64_C(18446744073709551615))

/* Values to test for integral types holding `void *' pointer.  */
#define INTPTR_MIN        (-2147483647-1)
#define INTPTR_MAX        (2147483647)
#define UINTPTR_MAX       (4294967295U)

/* Minimum for largest signed integral type.  */
#define INTMAX_MIN     (-INT64_C(9223372036854775807)-1)
/* Maximum for largest signed integral type.  */
#define INTMAX_MAX     (INT64_C(9223372036854775807))

/* Maximum for largest unsigned integral type.  */
#define UINTMAX_MAX        (UINT64_C(18446744073709551615))

/* Limits of `wchar_t'.  */
#define WCHAR_MIN     __WCHAR_MIN
#define WCHAR_MAX     __WCHAR_MAX