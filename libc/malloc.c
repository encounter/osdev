#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define MALLOC_DEBUG

#ifdef MALLOC_DEBUG
extern void kprint(char *message);
extern void kprint_uint32(uint32_t val);
#endif

static uintptr_t memory_start = 0x1000000;
static uintptr_t memory_end = 0x2000000; // obviously don't want to keep this
static bool initial_alloc = true;

struct chunk_header {
    struct chunk_header *next;
    struct chunk_header *prev;
    bool used;
    size_t size;
}; // __attribute__((packed))

_Static_assert(sizeof(struct chunk_header) == 0x10, "Header size wrong");

static bool try_reclaim(struct chunk_header *start, size_t size) {
    struct chunk_header *header = start;
    size_t unused = header->size;

    header = header->next;
    while (!header->used) {
        if (header->next == NULL) {
#ifdef MALLOC_DEBUG
            kprint("unused until end of chunks\n");
#endif
            start->next = NULL;
            return true;
        }

        unused += ((void *) header) - ((void *) header->prev);
#ifdef MALLOC_DEBUG
        kprint("found unused size "); kprint_uint32(unused); kprint("\n");
#endif
        if (unused >= size) {
            header->next->prev = start;
            start->next = header->next;
            return true;
        }
        header = header->next;
    }
    return false;
}

static uintptr_t find_unused_chunk(const size_t chunk_size) {
    if (initial_alloc) {
        if (memory_start + sizeof(struct chunk_header) + chunk_size > memory_end)
            return NULL;

        initial_alloc = false;

        // First chunk @ start of memory
#ifdef MALLOC_DEBUG
        kprint("new chunk @ start: "); kprint_uint32(chunk_size); kprint("\n");
#endif
        struct chunk_header *header = (struct chunk_header *) memory_start;
        memset(header, 0, sizeof(struct chunk_header));
        return memory_start;
    }

    struct chunk_header *header = (struct chunk_header *) memory_start;
    while (true) {
        if (!header->used) {
            if (header->size >= chunk_size) {
                // If unused chunk is large enough, go ahead and use it
#ifdef MALLOC_DEBUG
                kprint("reusing large enough chunk "); kprint_uint32(header->size); kprint(" for "); kprint_uint32(chunk_size); kprint("\n");
#endif
                return (uintptr_t) header;
            } else if (header->next != NULL && try_reclaim(header, chunk_size)) {
                // If there's a next chunk, and we're able to reclaim enough unused space
                return (uintptr_t) header;
            }
        }

        // Allocate new chunk if end
        if (header->next == NULL) {
#ifdef MALLOC_DEBUG
            kprint("alloc new chunk size: "); kprint_uint32(chunk_size); kprint("\n");
#endif
            uintptr_t next = (uintptr_t) ((void *) header) + sizeof(struct chunk_header) + header->size;
            if (next + chunk_size > memory_end) break; // Don't overcommit new chunk

            struct chunk_header *next_header = (struct chunk_header *) next;
            memset(next_header, 0, sizeof(struct chunk_header));
            header->next = next_header;
            next_header->prev = header;
            return next;
        }

        // Find space in between chunks
        uintptr_t end_of_chunk = (uintptr_t) header + sizeof(struct chunk_header) + header->size;
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
            return end_of_chunk;
        }

        header = header->next;
    }

    return NULL;
}

void *malloc(size_t size) {
    uintptr_t chunk = find_unused_chunk(size);
    if (chunk == NULL) {
        return NULL;
    }

    struct chunk_header *header = (struct chunk_header *) chunk;
    header->used = true;
    header->size = size;
    return ((void *) chunk) + sizeof(struct chunk_header);
}

void free(void *ptr) {
    if (ptr == NULL) return;
    struct chunk_header *header = (struct chunk_header *) (ptr - sizeof(struct chunk_header));
    header->used = false;
#ifdef MALLOC_DEBUG
    kprint("freeing chunk w/ size "); kprint_uint32(header->size); kprint("\n");
#endif
}

void print_chunk_debug(void *ptr) {
#ifdef MALLOC_DEBUG
    struct chunk_header *header = (struct chunk_header *) (ptr - sizeof(struct chunk_header));
    kprint("chunk @ "); kprint_uint32((uintptr_t) header);
    kprint(" | next: "); kprint_uint32((uintptr_t) header->next);
    kprint(", prev: "); kprint_uint32((uintptr_t) header->prev);
    kprint(", used: "); kprint_uint32((uint32_t) header->used);
    kprint(", size: "); kprint_uint32((uint32_t) header->size);
    kprint("\n");
#endif
}