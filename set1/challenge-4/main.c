#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned char ascii_hex_to_byte(char c) {
  return (c >= '0' && c <= '9') ? c - '0'
         : (c >= 'a' && c <= 'f') ? 10 + (c - 'a')
         : (c >= 'A' && c <= 'F') ? 10 + (c - 'A')
                                  : 0;
}

int main() {
  char lineBuf[60 + 1 + 1]; // 60 chars + newline + null term
  unsigned char msgBuf[sizeof(lineBuf) >> 1];
  unsigned char bestMsg[sizeof(lineBuf) >> 1];
  uint8_t key = 0;
  uint32_t bestScore = 0, bestKey = 0;

  FILE *fr = fopen("4.txt", "r");
  if (fr == NULL) {
    fprintf(stderr, "unable to open file.\n");
    exit(-1);
  }

  while (fgets(lineBuf, sizeof(lineBuf), fr)) {
    lineBuf[strcspn(lineBuf, "\n")] = '\0'; // Properly null-terminate by removing newline
    uint32_t lineBestScore = 0;
    uint8_t lineBestKey = 0;
    unsigned char lineBestMsg[sizeof(lineBuf) >> 1] = {0};
    for (size_t i = 0; i <= 255; ++i) {
      key = i;
      uint32_t score = 0;
      for (size_t j = 0; j < strlen(lineBuf) - 1; j += 2) {
        char byte = ascii_hex_to_byte(lineBuf[j]) << 4 |
                    ascii_hex_to_byte(lineBuf[j + 1]);
        char c = byte ^ key;
      int points = 0;
      switch (tolower(c)) {
          case 'e': points += 13; break;
          case 't': points += 9; break;
          case 'a': points += 8; break;
          case 'o': points += 8; break;
          case 'i': points += 7; break;
          case 'n': points += 7; break;
          case 's': points += 6; break;
          case 'h': points += 6; break;
          case 'r': points += 6; break;
          case 'd': points += 4; break;
          case 'l': points += 4; break;
          case 'u': points += 3; break;
          case ' ': points += 2; break;  // Also scoring space can help in identifying correct English phrases
      }
      score += points;
        msgBuf[j >> 1] = c;
      }
      if (score > lineBestScore) {
        lineBestScore = score;
        lineBestKey = key;
        strncpy((char *)lineBestMsg, (char *)msgBuf, sizeof(lineBestMsg));
        lineBestMsg[sizeof(lineBestMsg) - 1] = '\0'; // Ensure null termination
      }
    }
    if (lineBestScore > bestScore) {
      bestScore = lineBestScore;
      bestKey = lineBestKey;
      strncpy((char *)bestMsg, (char *)lineBestMsg, sizeof(bestMsg));
      bestMsg[sizeof(bestMsg) - 1] = '\0'; // Ensure null termination
    }
  }

  printf("Best Key is 0x%02X\n", bestKey);
  printf("Best message is: %s\n", bestMsg);

  fclose(fr);
  return 0;
}

