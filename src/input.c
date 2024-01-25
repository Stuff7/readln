#include "input.h"
#include <stdio.h>

#define ENTER '\n'
#define BACKSPACE 127
#define ESC 27
#define RIGHT 'C'
#define LEFT 'D'
#define CLEAR "\33[2K"

#ifdef __unix__
#include <termios.h>
#include <unistd.h>

char getch() {
  char buf = 0;
  struct termios old = {0};
  fflush(stdout);

  if (tcgetattr(0, &old) < 0) {
    perror("tcsetattr()");
  }

  old.c_lflag &= ~ICANON;
  old.c_lflag &= ~ECHO;
  old.c_cc[VMIN] = 1;
  old.c_cc[VTIME] = 0;

  if (tcsetattr(0, TCSANOW, &old) < 0) {
    perror("tcsetattr ICANON");
  }

  if (read(0, &buf, 1) < 0) {
    perror("read()");
  }

  old.c_lflag |= ICANON;
  old.c_lflag |= ECHO;

  if (tcsetattr(0, TCSADRAIN, &old) < 0) {
    perror("tcsetattr ~ICANON");
  }

  return buf;
}
#elif defined(_WIN32) || defined(WIN32)
char getch() {
  fprintf(stderr, "Implement getch for Windows\n");
  exit(EXIT_FAILURE);
}
#endif

const char handle_arrows() {
  if (getch() == '[') {
    return getch();
  }

  return 0;
}

void insertch(u8 *buf, usize *len, usize idx, u8 val) {
  for (int i = *len; i > idx; i--) {
    buf[i] = buf[i - 1];
  }
  buf[idx] = val;
  *len += 1;
}

void removech(u8 *buf, usize *len, usize idx) {
  if (!idx || !*len) {
    return;
  }

  for (int i = idx; i < *len - 1; i++) {
    buf[i] = buf[i + 1];
  }

  *len -= 1;
  buf[*len] = 0;
}

usize readln(u8 *buf, usize len) {
  char ch = 0;
  usize pos = 0;
  usize str_len = 0;

  while (1) {
    ch = getch();

    switch (ch) {
      case ENTER:
        printf("\n");
        return str_len;
      case BACKSPACE:
        if (pos) {
          removech(buf, &str_len, --pos);
        }
        break;
      case ESC:
        switch (handle_arrows()) {
          case LEFT:
            if (pos) {
              pos--;
            }
            break;
          case RIGHT:
            if (pos < str_len) {
              pos++;
            }
            break;
        }
        break;
      default:
        if (pos < len - 1) {
          insertch(buf, &str_len, pos++, ch);
        }
        break;
    }

    printf("%s\r%s\r\033[%ldC", CLEAR, buf, pos);
  }

  return str_len;
}
