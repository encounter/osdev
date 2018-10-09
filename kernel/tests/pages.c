#include "tests.h"
#include "../kmalloc.h"

#include <stdio.h>
#include <string.h>

#define ASSERT_EQ(expected, actual) if ((expected) != (actual)) { \
                                      printf("Failed on line %d. Expected: %u. Actual: %u\n", \
                                             __LINE__, (uint32_t) expected, (uint32_t) actual); \
                                      return false; \
                                    }

#define ASSERT_NE(not_expected, actual) if ((not_expected) == (actual)) { \
                                          printf("Failed on line %d. Non expected actual value: %u\n", \
                                                 __LINE__, (uint32_t) actual); \
                                          return false; \
                                        }

extern volatile uintptr_t mmap_end;

#define PAGE_SIZE 0x400000

bool page_table_test() {
    uintptr_t mmap_before = mmap_end;
    printf("Before: %u\n", mmap_before);
    void *ptr = kmalloc(PAGE_SIZE - 0x10);
    ASSERT_NE(NULL, ptr);
    ASSERT_EQ(mmap_before + PAGE_SIZE, mmap_end);
    printf("After: %u\n", mmap_end);
    memset(ptr, 0xFF, PAGE_SIZE - 0x10);
//    kfree(ptr);
    return true;
}