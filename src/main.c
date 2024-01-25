#include "input.h"
#include <stdlib.h>

#define BUF_LEN 256

int main() {
  u8 *buf = calloc(BUF_LEN, 1);
  usize len = readln(buf, BUF_LEN);

  printf("Character count: %ld\nOutput: %s\n", len, buf);
  free(buf);

  return 0;
}
