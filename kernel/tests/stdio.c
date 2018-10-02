#include <common.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define FAIL() { \
    fail_line = __LINE__; \
    goto fail; \
}

const char filename[] = "README";
const char test_str[] = "fwrite";

extern int ff_abrt_line;

static bool stat_test() {
    struct stat st;
    bool ret = !(stat(filename, &st) || !st.st_size);
    printf("stat_test: %s.\n", ret ? "passed" : "failed");
    return ret;
}

static bool fread_test() {
    bool ret = true;
    int fail_line = 0;

    struct stat st;
    FILE *file = {0};
    void *buff = NULL;

    if (stat(filename, &st) || !st.st_size) FAIL();
    printf("Opening %s with size %llu...\n", filename, st.st_size);
    file = fopen(filename, "r");
    if (ferror(file)) FAIL();

    if ((buff = malloc((size_t) st.st_size + 1)) == NULL) FAIL();
    size_t read = fread(buff, (uint32_t) st.st_size, 1, file);
    if (ferror(file) || !read) FAIL();

    printf("Checking file contents...\n");
    if (strncmp(buff, "mkdir", 5) != 0) FAIL();

    printf("fread_test: passed.\n");
    goto end;

    fail:
    ret = false;
    printf("fread_test: failed on line %d. (errno %d)\n", fail_line, errno);

    end:
    if (file != NULL) fclose(file);
    free(buff);
    return ret;
}

static bool fwrite_test() {
    bool ret = true;
    int fail_line = 0;

    struct stat st;
    FILE *file = {0};
    void *rbuff = NULL;
    void *wbuff = NULL;

    if (stat(filename, &st) || !st.st_size) FAIL();
    printf("Opening %s for updating... %lli\n", filename, st.st_size);
    file = fopen(filename, "r+");
    if (ferror(file)) FAIL();

    if ((rbuff = malloc((size_t) st.st_size + 1)) == NULL) FAIL();
    if (!fread(rbuff, (uint32_t) st.st_size, 1, file) || ferror(file)) FAIL();
    if (fseek(file, 0, SEEK_SET) || ferror(file)) FAIL();
    if (!fwrite(test_str, sizeof(test_str) - 1, 1, file) || ferror(file)) FAIL();
    if (fclose(file)) FAIL();

    if (stat(filename, &st) || !st.st_size) FAIL();
    printf("Re-open %s for updating... %lli\n", filename, st.st_size);
    file = fopen(filename, "r+");
    if (ferror(file)) FAIL();
    if ((wbuff = malloc(sizeof(test_str))) == NULL) FAIL();
    if (fseek(file, 0, SEEK_SET) || ferror(file)) FAIL();
    if (!fread(wbuff, sizeof(test_str), 1, file) || ferror(file)) FAIL();
    if (strncmp(wbuff, test_str, sizeof(test_str) - 1) != 0) FAIL();
    if (fseek(file, 0, SEEK_SET) || ferror(file)) FAIL();
    if (!fwrite(rbuff, sizeof(test_str) - 1, 1, file) || ferror(file)) FAIL();

    printf("fwrite_test: passed.\n");
    goto end;

    fail:
    ret = false;
    printf("fwrite_test: failed on line %d. (errno %d, ff_abrt_line %d)\n", fail_line, errno, ff_abrt_line);

    end:
    if (file != NULL) {
        fflush(file);
        fclose(file);
    }
    free(rbuff);
    free(wbuff);
    return ret;
}

bool stdio_test() {
    return stat_test() &&
           fread_test() &&
           fwrite_test();
}