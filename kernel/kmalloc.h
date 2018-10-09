#pragma once

#include <common.h>

void *kmalloc(size_t size);

void *krealloc(void *ptr, size_t new_size);

void kfree(void *ptr);

void print_chunk_debug(void *ptr, bool recursive);