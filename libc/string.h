#pragma once

#include <common.h>

#define ALIGN(x, a)              __ALIGN_MASK(x,(__typeof__(x))(a)-1)
#define __ALIGN_MASK(x, mask)    (((x)+(mask))&~(mask))

void *memcpy(void *restrict destination, const void *restrict source, size_t num);

void *memmove(void *destination, const void *source, size_t num);

void *memset(void *ptr, int value, size_t num);

int memcmp(const void *s1, const void *s2, size_t n);

void *memchr(const void *src, int c, size_t n);

size_t strlen(const char *str);

size_t strnlen(const char *s, size_t n);

int strcmp(const char *str1, const char *str2);

int strncmp(const char *str1, const char *str2, size_t num);

char *strcpy(char *destination, const char *source);

char *strncpy(char *restrict s1, const char *restrict s2, size_t n);

char *strdup(char *str);