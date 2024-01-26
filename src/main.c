#include "input.h"

#define BUF_LEN 256

int main() {
  u8 buf[BUF_LEN];
  usize len = readln(buf, BUF_LEN);
  printf("Character count: %ld\nOutput: %s\n", len, buf);

  return 0;
}
