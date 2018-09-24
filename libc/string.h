#pragma once

#include <stddef.h>
#include <stdint.h>

#define size_t uint32_t

void *memcpy(void *restrict destination, const void *restrict source, size_t num);

void *memmove(void *destination, const void *source, size_t num);

void *memset(void *ptr, int value, size_t num);

int memcmp(const void *s1, const void *s2, size_t n);

size_t strlen(const char *str);

int strcmp(const char *str1, const char *str2);

int strncmp(const char *str1, const char *str2, size_t num);

char *strcpy(char *destination, const char *source);

char *strncpy(char *restrict s1, const char *restrict s2, size_t n);

char *strdup(char *str);