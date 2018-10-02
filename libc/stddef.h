#pragma once

#include <stdint.h>

#define NULL ((void *) 0)

typedef __PTRDIFF_TYPE__ ptrdiff_t;
typedef uint32_t size_t; // FIXME __SIZE_TYPE__ wrong on macOS?
typedef __WCHAR_TYPE__ wchar_t;
