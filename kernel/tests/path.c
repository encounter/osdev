#include <common.h>
#include <string.h>
#include <stdio.h>

#define CHECK(s1, s2, len) { \
    if (strncmp(s1, s2, len) != 0) { \
        printf("path_test fail line %d: Expected: %s, actual: %s\n", __LINE__, s2, s1); \
        return false; \
    } \
}

bool path_test() {
    char buf[512];
    size_t len = sizeof(buf);

    path_append(buf, "/test", "123", len);
    CHECK(buf, "/test/123", len);

    path_append(buf, "/test/", "/123", len);
    CHECK(buf, "/123", len);

    path_append(buf, "/test/", "/123/test 456/../test 567", len);
    CHECK(buf, "/123/test 567", len);

    path_append(buf, "/test//", "123/", len);
    CHECK(buf, "/test/123/", len);

    path_append(buf, "/test", "..", len);
    CHECK(buf, "/", len);

    path_append(buf, "/", "..", len);
    CHECK(buf, "/", len);

    path_append(buf, "/test/..", "123", len);
    CHECK(buf, "/123", len);

    path_append(buf, "/test/", "123/..", len);
    CHECK(buf, "/test/", len);

    path_append(buf, "/test/.", "123/./456", len);
    CHECK(buf, "/test/123/456", len);

    path_append(buf, "test 1", "test 2", len);
    CHECK(buf, "test 1/test 2", len);

    path_append(buf, "/1/2/3/4/5/6/7/8/9/../9", "10/", len);
    CHECK(buf, "/1/2/3/4/5/6/7/8/9/10/", len);

    // FIXME later
//    path_append(buf, "/1/2/3/4/5/./6/./7/8/9/../9", "10/.", len);
//    CHECK(buf, "/1/2/3/4/5/6/7/8/9/10/", len);

    path_append(buf, "/dev/", "", len);
    CHECK(buf, "/dev/", len);

    path_append(buf, "/dev/", ".null", len);
    CHECK(buf, "/dev/.null", len);

    path_append(buf, "", ".config", len);
    CHECK(buf, ".config", len);

    path_append(buf, "", NULL, len);
    CHECK(buf, "", len);

    path_append(buf, "test", NULL, len);
    CHECK(buf, "test", len);

    path_append(buf, "/test", ".", len);
    CHECK(buf, "/test", len);

    printf("path_test: passed\n");
    return true;
}