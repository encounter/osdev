#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define _noreturn _Noreturn
#define static_assert _Static_assert

#ifdef __GNUC__
#define _unused   __attribute__((unused))
#define _packed   __attribute__((packed))
#endif

#define off_t int64_t