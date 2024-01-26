#include "input.h"
#include <stdio.h>

#define ENTER '\n'
#define BACKSPACE 127
#define ESC 27
#define CLEAR "\33[2K"

typedef enum {
  Special,
  Char,
  Enter,
  Backspace,
  ArrowRight,
  ArrowLeft,
  CtrlArrowRight,
  CtrlArrowLeft,
} Key;

#define ESC_SEQ_MAX_LEN 5
typedef struct {
  Key key;
  u8 value[ESC_SEQ_MAX_LEN];
  int len;
} EscapeSequence;

#define ESC_SEQ_LEN 6
const static EscapeSequence escSeqList[ESC_SEQ_LEN] = {
    {ArrowRight, "[C", 2},
    {ArrowLeft, "[D", 2},
    {CtrlArrowRight, "[1;5C", 5},
    {CtrlArrowLeft, "[1;5D", 5},
};

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

const Key readKey(u8 *ret) {
  u8 ch = getch();

  switch (ch) {
    case ESC:
      break;
    case '\n':
      return Enter;
    case 127:
      return Backspace;
    default:
      *ret = ch;
      return Char;
  }

  int pos = 0;
  while (pos < ESC_SEQ_MAX_LEN) {
    ch = getch();

    for (int i = 0; i < ESC_SEQ_LEN; i++) {
      const EscapeSequence *seq = &escSeqList[i];

      if (pos == seq->len) {
        continue;
      }

      if (ch == seq->value[pos] && seq->len - 1 == pos) {
        return seq->key;
      }
    }

    pos++;
  }

  return Special;
}
#elif defined(_WIN32) || defined(WIN32)
#include <Windows.h>
#include <stdlib.h>

static int arrowState = 0;
static WORD wVirtualKeyCode = 0;

char getch() {
  HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
  INPUT_RECORD irInputRecord;
  DWORD dwEventsRead;

  if (arrowState) {
    if (arrowState == 1) {
      arrowState++;
      return '[';
    }

    arrowState = 0;
    switch (wVirtualKeyCode) {
      case VK_UP:
        return UP;
      case VK_DOWN:
        return DOWN;
      case VK_LEFT:
        return LEFT;
      case VK_RIGHT:
        return RIGHT;
    }
  }

  while (ReadConsoleInputA(hStdin, &irInputRecord, 1, &dwEventsRead)) {
    if (irInputRecord.EventType == KEY_EVENT && irInputRecord.Event.KeyEvent.bKeyDown) {
      if (irInputRecord.Event.KeyEvent.uChar.AsciiChar != 0) {
        switch (irInputRecord.Event.KeyEvent.uChar.AsciiChar) {
          case VK_RETURN:
            return ENTER;
          case VK_BACK:
            return BACKSPACE;
        }

        return irInputRecord.Event.KeyEvent.uChar.AsciiChar;
      } else {
        wVirtualKeyCode = irInputRecord.Event.KeyEvent.wVirtualKeyCode;
        arrowState = 1;
        return '\033';
      }
    }
  }

  return '\0';
}

const SpecialKey escapeSequence() {
  fprintf(stderr, "Implement escapeSequence for Windows\n");
  exit(1);
}
#endif

void insertch(u8 *buf, usize *len, usize idx, u8 val) {
  for (int i = *len; i > idx; i--) {
    buf[i] = buf[i - 1];
  }
  buf[idx] = val;
  *len += 1;
}

void removech(u8 *buf, usize *len, usize idx) {
  if (!*len) {
    return;
  }

  for (int i = idx; i < *len - 1; i++) {
    buf[i] = buf[i + 1];
  }

  *len -= 1;
  buf[*len] = 0;
}

usize readln(u8 buf[], usize len) {
  u8 ch = 0;
  usize pos = 0;
  usize str_len = 0;
  Key key;

  while (1) {
    key = readKey(&ch);

    switch (key) {
      case Enter:
        printf("\n");
        return str_len;
      case Backspace:
        if (pos) {
          removech(buf, &str_len, --pos);
        }
        break;
      case ArrowLeft:
        if (pos) {
          pos--;
        }
        break;
      case ArrowRight:
        if (pos < str_len) {
          pos++;
        }
        break;
      case CtrlArrowLeft:
        while (pos) {
          if (buf[--pos] == ' ') {
            break;
          }
        }
        break;
      case CtrlArrowRight:
        while (pos < str_len) {
          if (buf[++pos] == ' ') {
            break;
          }
        }
        break;
      case Special:
        break;
      default:
        if (pos < len - 1) {
          insertch(buf, &str_len, pos++, ch);
        }
        break;
    }

    printf("%s\r%s\r", CLEAR, buf);
    if (pos) {
      printf("\033[%ldC", pos);
    }
  }

  return str_len;
}
