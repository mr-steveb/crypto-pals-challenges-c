#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

const static char encrypted[] =
    "1b37373331363f78151b7f2b783431333d78397828372d363c78373e783a393b3736";

unsigned char ascii_hex_to_byte(char c) {
  return (c >= '0' && c <= '9')   ? c - '0'
         : (c >= 'a' && c <= 'f') ? 10 + (c - 'a')
         : (c >= 'A' && c <= 'F') ? 10 + (c - 'A')
                                  : 0;
}

typedef struct {
  char *string;
  unsigned char *decryptedMsg;
  float score;
} TableRow;

int main() {

  const size_t ROWS = 256;
  TableRow table[ROWS];
  size_t encryptedLen = sizeof(encrypted);
  size_t msgLen = sizeof(encrypted) >> 1;
  unsigned char key = 0x0;
  char msg[msgLen + 1];
  msg[msgLen] = '\0';
  char *bestMsg = malloc(msgLen + 1);
  unsigned char best_key;
  float best_score = -1.0F;

  for (size_t i = 0; i < ROWS; ++i) {
    table[i].string = (char *)malloc(18 * sizeof(char));
    table[i].decryptedMsg =
        (unsigned char *)malloc(msgLen * sizeof(unsigned char));
    table[i].score = -1.0F;
  }

  char byte = 0;
  char buf[255];
  buf[17] = '\0';

  for (size_t i = 0; i <= 255; ++i) {
    key = i;
    sprintf((char *)buf, "Message No. %lu: ", i + 1);
    float score = 0.0;
    for (size_t j = 0; j < encryptedLen; j += 2) {
      byte = ascii_hex_to_byte(encrypted[j]) << 4 |
             ascii_hex_to_byte(encrypted[j + 1]);
      msg[j >> 1] = byte ^ (char)key;
    
      if (strchr("etaoin shrdlu", tolower(msg[j>>1]))) {
        score += 1.0; // Simplified scoring for demonstration
      }
     }
    
    size_t bufLen = strlen(buf);
    if (bufLen > 18) {
      fprintf(stderr, "possible overflow attempt. Exiting\n");
      exit(1);
    }

    table[i].score = score;
    if (score > best_score) {
      best_key = key;
      best_score = score;
      strncpy(bestMsg, msg, msgLen+1);
      bestMsg[msgLen] = '\0';
    }
    strncpy(table[i].string, buf, bufLen + 1);
    table[i].string[bufLen] = '\0';
    memcpy(table[i].decryptedMsg, msg, msgLen);
    printf("%s %.2f  ", table[i].string, table[i].score);
    for (size_t k = 0; k < msgLen; k++)
      printf("%02x", table[i].decryptedMsg[k]);
    printf(" 0x%02X", key);
    printf("\n");

  }

  printf("%s",bestMsg);
  printf("\n");
  printf("best key: 0x%02X\n",(char)best_key);
  for (size_t i = 0; i < ROWS; ++i) {
    free(table[i].string);
    free(table[i].decryptedMsg);
  }
  free(bestMsg);

  return 0;
}
