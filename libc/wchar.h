#pragma once

#include <common.h>

#define IS_CODEUNIT(c) ((unsigned)(c)-0xdf80 < 0x80)

typedef __WCHAR_TYPE__ wchar_t;

int wctomb(char *s, wchar_t wc);

size_t wcrtomb(char *__restrict, wchar_t);