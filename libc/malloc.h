#include <string.h>

void* malloc(size_t size);

void free(void *ptr);

void print_chunk_debug(void *ptr);