#include "malloc.h"
#include "stdio.h"

#include <string.h>
#include <limits.h>

// #define MALLOC_DEBUG

void *malloc_memory_start = (void *) 0x1000000; // 1 MiB, start of x86 upper memory
void *malloc_memory_end = NULL;
static bool initial_alloc = true;

struct chunk_header {
    struct chunk_header *next;
    struct chunk_header *prev;
    size_t size;
    bool used;
};

_Static_assert(sizeof(struct chunk_header) % 4 == 0, "chunk_header is misaligned");

static bool try_reclaim(struct chunk_header *start, size_t size) {
    struct chunk_header *current_chunk = start;
    if (current_chunk->next == NULL) return false;

    size_t unused = current_chunk->size;
    current_chunk = current_chunk->next;
    while (!current_chunk->used) {
        struct chunk_header *next_chunk = current_chunk->next;
        if (next_chunk == NULL) {
#ifdef MALLOC_DEBUG
            kprint("unused until end of chunks\n");
#endif
            start->next = NULL;
            return true;
        }

        unused += ((void *) current_chunk) - ((void *) current_chunk->prev);
#ifdef MALLOC_DEBUG
        kprint("found unused size "); kprint_uint32(unused); kprint("\n");
#endif
        if (unused >= size) {
            next_chunk->prev = start;
            start->next = next_chunk;
            return true;
        }
        current_chunk = next_chunk;
    }
    return false;
}

static void *find_unused_chunk(const size_t chunk_size) {
    if (initial_alloc) {
        if (malloc_memory_start + sizeof(struct chunk_header) + chunk_size > malloc_memory_end)
            return NULL;

        initial_alloc = false;

        // First chunk @ start of memory
#ifdef MALLOC_DEBUG
        kprint("new chunk @ start: "); kprint_uint32(chunk_size); kprint("\n");
#endif
        struct chunk_header *header = malloc_memory_start;
        memset(header, 0, sizeof(struct chunk_header));
        return malloc_memory_start;
    }

    struct chunk_header *header = malloc_memory_start;
    while (true) {
        if (!header->used) {
            if (header->size >= chunk_size) {
                // Unused chunk is large enough, go ahead and use it
#ifdef MALLOC_DEBUG
                kprint("reusing large enough chunk "); kprint_uint32(header->size); kprint(" for "); kprint_uint32(chunk_size); kprint("\n");
#endif
                return header;
            } else if (try_reclaim(header, chunk_size)) {
                // There's a next chunk, and we were able to reclaim enough unused space
                return header;
            }
        }

        // Allocate new chunk if end
        if (header->next == NULL) {
#ifdef MALLOC_DEBUG
            kprint("alloc new chunk size: "); kprint_uint32(chunk_size); kprint("\n");
#endif
            void *next = (void *) header + sizeof(struct chunk_header) + ALIGN(header->size, 4);
            if (next + chunk_size > malloc_memory_end) break; // Don't overcommit new chunk

            struct chunk_header *next_header = memset(next, 0, sizeof(struct chunk_header));
            header->next = next_header;
            next_header->prev = header;
            return next;
        }

        // Find space in between chunks
        uintptr_t end_of_chunk = (uintptr_t) header + sizeof(struct chunk_header) + ALIGN(header->size, 4);
        uintptr_t unused_size = (uintptr_t) header->next - end_of_chunk;
        if (unused_size > chunk_size + sizeof(struct chunk_header)) {
#ifdef MALLOC_DEBUG
            kprint("found size between chunk: "); kprint_uint32(unused_size); kprint(" > "); kprint_uint32(chunk_size); kprint("\n");
            kprint("chunk 1: "); kprint_uint32((uintptr_t) header); kprint(" | chunk 2: "); kprint_uint32((uintptr_t) header->next); kprint("\n");
#endif
            struct chunk_header *next_header = (struct chunk_header *) end_of_chunk;
            memset(next_header, 0, sizeof(struct chunk_header));
            next_header->next = header->next;
            next_header->prev = header;
            header->next->prev = next_header;
            header->next = next_header;
            return (void *) end_of_chunk;
        }

        header = header->next;
    }

    return NULL;
}

void *malloc(size_t size) {
    void *chunk = find_unused_chunk(size);
    if (chunk == NULL) {
        return NULL;
    }

    struct chunk_header *header = chunk;
    header->used = true;
    header->size = size;
    return chunk + sizeof(struct chunk_header);
}

void free(void *ptr) {
    if (ptr == NULL) return;
    struct chunk_header *header = ptr - sizeof(struct chunk_header);
    header->used = false;
#ifdef MALLOC_DEBUG
    kprint("freeing chunk w/ size "); kprint_uint32(header->size); kprint("\n");
#endif
}

void *realloc(void *ptr, size_t new_size) {
    if (ptr == NULL) return malloc(new_size);
    struct chunk_header *header = ptr - sizeof(struct chunk_header);
    if (header->size >= new_size || try_reclaim(header, new_size)) {
        header->size = new_size;
        return ptr;
    }

    void *new = malloc(new_size);
    memcpy(new, ptr, header->size);
    free(ptr);
    return new;
}

// FIXME move everything below

const char *suffixes[7] = {
    "",
    "KB",
    "MB",
    "GB",
    "TB",
    "PB",
    "EB"
};

static unsigned digits(uint32_t n) {
    static uint32_t powers[10] = {
            0, 10, 100, 1000, 10000, 100000, 1000000,
            10000000, 100000000, 1000000000,
    };
    static unsigned max_digits[33] = {
            1, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 4, 5, 5,
            5, 6, 6, 6, 7, 7, 7, 7, 8, 8, 8, 9, 9, 9, 10, 10, 10,
    };
    unsigned bits = sizeof(n) * CHAR_BIT - __builtin_clz(n);
    unsigned digits = max_digits[bits];
    if (n < powers[digits - 1]) --digits;
    return digits;
}

static char *pretty_bytes(uint64_t bytes) {
    uint8_t s = 0;
    uint64_t count = bytes;
    while (count >= 1024 && s < 7) {
        s++;
        count /= 1024;
    }

    uint32_t len = digits((uint32_t) count) + 2;
    char *buf = malloc(len + 1);
    buf[snprintf(buf, len, "%llu%s", count, suffixes[s])] = 0;
    return buf;
}

void print_chunk_debug(void *ptr, bool recursive) {
    printf("memory_start = %p, end = %p\n", malloc_memory_start, malloc_memory_end);

    uint32_t used = 0;
    if (ptr == NULL) ptr = malloc_memory_start + sizeof(struct chunk_header);
    struct chunk_header *header = ptr - sizeof(struct chunk_header);
    do {
        char *str_size = pretty_bytes(header->size);
        printf("chunk @ %p | next: %p, prev: %p, used: %d, size: %s\n",
               header, header->next, header->prev, header->used, str_size);
        free(str_size);

        if (header->used) {
            if (header->next != NULL) {
                used += (uintptr_t) header->next - (uintptr_t) header;
            } else {
                used += sizeof(struct chunk_header) + header->size;
            }
        }
        if (recursive && header->next != NULL) header = header->next;
    } while (header->next != NULL);

    if (recursive) {
        uint32_t est_free = (uintptr_t) malloc_memory_end - (uintptr_t) malloc_memory_start - used;
        char *str_used = pretty_bytes(used);
        char *str_free = pretty_bytes(est_free);
        printf("used = %s, est. free = %s\n", str_used, str_free);
        free(str_used);
        free(str_free);
    }
}