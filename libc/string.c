#include <string.h>

// One measly byte at a time...
void *memcpy(void *destination, const void *source, size_t num) {
  for (int i = 0; i < num; i++)
    ((uint8_t *) destination)[i] = ((uint8_t *) source)[i];
  return destination;
}

void *memset(void *ptr, int value, size_t num) {
  for (int i = 0; i < num; i++)
    ((unsigned char *) ptr)[i] = (unsigned char) value;
  return ptr;
}

size_t strlen(const char *str) {
  size_t ret = 0, i = 0;
  while (str[i++] != NULL) ret++;
  return ret;
}
