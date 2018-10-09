#include "kmalloc.h"
#include "stdio.h"
#include "console.h"
#include "arch/x86/mmu.h"

#include <string.h>
#include <limits.h>

 #define MALLOC_DEBUG

#define PAGE_SIZE 0x400000
uintptr_t mmap_end = 0;
void *start_chunk;

// Initialized from multiboot
void *malloc_memory_start = NULL;
void *malloc_memory_end = NULL;
static bool initial_alloc = true;

typedef struct chunk_header chunk_header_t;

struct chunk_header {
    chunk_header_t *next;
    chunk_header_t *prev;
    size_t size;
    unsigned short used;
};

static_assert(sizeof(chunk_header_t) % 4 == 0, "chunk_header is misaligned");

static int try_reclaim(chunk_header_t *start, size_t size) {
    chunk_header_t *current_chunk = start;
    if (current_chunk->next == NULL) return false;

    size_t unused = current_chunk->size;
    current_chunk = current_chunk->next;
    while (!current_chunk->used) {
        chunk_header_t *next_chunk = current_chunk->next;
        if (next_chunk == NULL) {
            return 2; // Hit end of chunks
        }

        unused += (uintptr_t) next_chunk - (uintptr_t) current_chunk;
        // FIXME do i need ALIGN here?
        if (unused >= ALIGN(size, 4) + sizeof(chunk_header_t)) {
            next_chunk->prev = start;
            start->next = next_chunk;
            return 1;
        }
        current_chunk = next_chunk;
    }
    return 0;
}

static bool ensure_pages(void *new_end) {
    uintptr_t phys_end = (uintptr_t) virt_to_phys(new_end);
    fprintf(serial, "Ensuring pages up to %P -> %P\n", new_end, phys_end);
    while (mmap_end < phys_end) {
        uintptr_t next_page = mmap_end;
        if (next_page == 0) {
            next_page = ALIGN((uintptr_t) malloc_memory_start, PAGE_SIZE);
        }
        if (next_page + PAGE_SIZE > (uintptr_t) malloc_memory_end) {
            // Oops, out of memory
            return false;
        }
        page_table_set(next_page, (uintptr_t) phys_to_virt((void *) next_page), 0x83);
        mmap_end = next_page + PAGE_SIZE;
    }
    return true;
}

static chunk_header_t *find_unused_chunk(const size_t chunk_size) {
    if (initial_alloc) {
        if (malloc_memory_start == NULL)
            return NULL;

        start_chunk = phys_to_virt((void *) ALIGN((uintptr_t) malloc_memory_start, PAGE_SIZE));
        ensure_pages(start_chunk + sizeof(chunk_header_t) + chunk_size);

        initial_alloc = false;

        // First chunk @ start of memory
#ifdef MALLOC_DEBUG
        fprintf(serial, "new chunk @ start: %zu\n", chunk_size);
#endif
        chunk_header_t *header = start_chunk;
        memset(header, 0, sizeof(chunk_header_t));
        return header;
    }

    chunk_header_t *header = start_chunk;
    while (true) {
        if (!header->used) {
            if (header->size >= chunk_size) {
                // Unused chunk is large enough, go ahead and use it
#ifdef MALLOC_DEBUG
                fprintf(serial, "reusing large enough chunk %P, %zu > %zu\n", header, header->size, chunk_size);
#endif
                return header;
            }
            int ret = try_reclaim(header, chunk_size);
            if (ret == 1) {
                // There's a next chunk, and we were able to reclaim enough unused space
                return header;
            } else if (ret == 2) {
                // We hit the end of allocated chunks, so do some housekeeping
                if (header->prev != NULL) {
                    header->prev->next = NULL;
                    header = header->prev;
                } else {
                    // Should not be true, unless something crazy happened
                    header->next = NULL;
                }
            }
        }

        // Allocate new chunk if end
        if (header->next == NULL) {
            void *next = (void *) header + sizeof(chunk_header_t) + ALIGN(header->size, 4);
            if (!ensure_pages(next + chunk_size + sizeof(chunk_header_t))) return NULL;
#ifdef MALLOC_DEBUG
            fprintf(serial, "alloc new chunk %P size: %zu\n", next, chunk_size);
#endif

            chunk_header_t *next_header = memset(next, 0, sizeof(chunk_header_t));
            header->next = next_header;
            next_header->prev = header;
            return next;
        }

        // Find space in between chunks
        uintptr_t end_of_chunk = (uintptr_t) header + sizeof(chunk_header_t) + ALIGN(header->size, 4);
        if (end_of_chunk > (uintptr_t) header->next) {
            panic("kmalloc: misaligned chunk %P (size %zu). Expected end: %P, actual next: %P\n",
                  header, header->size, end_of_chunk, header->next);
        }
        uintptr_t unused_size = (uintptr_t) header->next - end_of_chunk;
        if (unused_size > chunk_size + sizeof(chunk_header_t)) {
#ifdef MALLOC_DEBUG
            fprintf(serial, "found size between chunk: %zu > %zu\n", unused_size, chunk_size);
            fprintf(serial, "  chunk 1: %P (size %zu) | chunk 2: %P\n", header, header->size, header->next);
#endif
            chunk_header_t *next_header = (chunk_header_t *) end_of_chunk;
            memset(next_header, 0, sizeof(chunk_header_t));
            next_header->next = header->next;
            next_header->prev = header;
            header->next->prev = next_header;
            header->next = next_header;
            return next_header;
        }

        header = header->next;
    }

    return NULL;
}

void *kmalloc(size_t size) {
#ifdef MALLOC_DEBUG
    fprintf(serial, "malloc called with size %zu\n", size);
#endif
    if (size == 0) return NULL;

    chunk_header_t *header = find_unused_chunk(size);
    if (header == NULL) return NULL;

    header->used = true;
    header->size = size;
    return (void *) header + sizeof(chunk_header_t);
}

void kfree(void *ptr) {
    if (ptr == NULL) return;
    chunk_header_t *header = ptr - sizeof(chunk_header_t);
    header->used = false;
#ifdef MALLOC_DEBUG
    fprintf(serial, "freed chunk %P w/ size %zu\n", header, header->size);
#endif
}

void *krealloc(void *ptr, size_t new_size) {
    if (ptr == NULL) return kmalloc(new_size);

    chunk_header_t *header = ptr - sizeof(chunk_header_t);
    if (header->size >= new_size || try_reclaim(header, new_size)) {
        header->size = new_size;
        return ptr;
    }

    void *new = kmalloc(new_size);
    memcpy(new, ptr, header->size);
    kfree(ptr);
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

static char *pretty_bytes(uint64_t bytes, char *buf, size_t len) {
    uint8_t s = 0;
    uint64_t count = bytes;
    while (count >= 1024 && s < 7) {
        s++;
        count /= 1024;
    }

    buf[snprintf(buf, len, "%llu%s", count, suffixes[s])] = 0;
    return buf;
}

void print_chunk_debug(void *ptr, bool recursive) {
    printf("memory_start = %P, end = %P, mmap_end = %P\n", malloc_memory_start, malloc_memory_end, mmap_end);

    uint32_t used = 0;
    if (ptr == NULL) ptr = start_chunk + sizeof(chunk_header_t);
    chunk_header_t *header = ptr - sizeof(chunk_header_t);
    char str_size[10];
    do {
        pretty_bytes(header->size, str_size, 10);
        printf(". %P | next: %P, prev: %P, used: %d, size: %s\n",
               header, header->next, header->prev, header->used, str_size);

        if (header->used) {
            if (header->next != NULL) {
                used += (uintptr_t) header->next - (uintptr_t) header;
            } else {
                used += sizeof(chunk_header_t) + header->size;
            }
        }
        if (recursive && header->next != NULL) {
            header = header->next;
        } else {
            break;
        }
    } while (true);

    if (recursive) {
        uint32_t est_free = (uintptr_t) malloc_memory_end - (uintptr_t) malloc_memory_start - used;
        char str_used[10], str_free[10];
        pretty_bytes(used, str_used, 10);
        pretty_bytes(est_free, str_free, 10);
        printf("used = %s, est. free = %s\n", str_used, str_free);
    }
}