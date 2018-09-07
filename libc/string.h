#pragma once

#include <stdint.h>

#define size_t uint32_t
#define NULL 0

void *memcpy(void *destination, const void *source, size_t num);

void *memset(void *ptr, int value, size_t num);

size_t strlen(const char *str);