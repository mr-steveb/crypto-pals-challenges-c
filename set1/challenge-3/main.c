#include <ctype.h>
#include <stdio.h>
#include <string.h>

static const char encMsg[] =
    "\x1b\x37\x37\x33\x31\x36\x3f\x78\x15\x1b\x7f\x2b\x78\x34\x31\x33\x3d\x78"
    "\x39\x78\x28\x37\x2d\x36\x3c\x78\x37\x3e\x78\x3a\x39\x3b\x37\x36";

int main(int argc, char *argv[]) {
  int i = 0;
  int key = 0;
  int best_key = -1;
  int best_score = -1;

  for (key = 0; key <= 255; ++key) {
    int score = 0;
    for (i = 0; i < strlen(encMsg); ++i) {
      char c = encMsg[i] ^ (unsigned char)key;
      if (strchr("etaoin shrdlu", tolower(c)))
        ++score;
    }
    if (score > best_score) {
      best_key = key;
      best_score = score;
    }
  }
  for (i = 0; i < strlen(encMsg); ++i) {
    printf("%c", encMsg[i] ^ (char)best_key);
  }
  printf("\n");
  return 0;
}
