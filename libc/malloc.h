#pragma once

#include <string.h>
#include <stdbool.h>

void *malloc(size_t size);

void *realloc(void *ptr, size_t new_size);

void free(void *ptr);

void print_chunk_debug(void *ptr, bool recursive);