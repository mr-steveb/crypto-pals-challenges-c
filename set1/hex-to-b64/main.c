#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int hex_char_to_bin(char c) {
  if (c >= '0' && c <= '9') {
    return c - '0';
  } else if (c >= 'a' && c <= 'f') {
    return 10 + c - 'a';
  } else if (c >= 'A' && c <= 'F') {
    return 10 + c - 'A';
  } else {
    return -1;
  }
}

void hex_str_to_bin(const char *hexStr, uint8_t *binData, size_t binSize) {

  for (size_t i = 0; i < binSize; i++) {
    uint8_t highNibble = hex_char_to_bin(hexStr[i << 1]);
    uint8_t lowNibble = hex_char_to_bin(hexStr[(i << 1) + 1]);
    if (highNibble >= 0 && lowNibble >= 0) {
      binData[i] = (highNibble << 4) | lowNibble;
    } else {
      fprintf(stderr, "Invalid Hex character.\n");
      exit(1);
    }
  }
}

void trim_newline(unsigned char *string) {

  for (int i = 0; string[i] != '\0'; i++) {
    if (string[i] == '\n') {
      string[i] = '\0';
      break; // Exit the loop after replacing the first newline character
    }
  }
}

uint8_t *hex_to_b64(uint8_t *binBuffer, size_t binSize, size_t base64BufLen,
                    uint8_t flag) {

  const uint32_t mask = 0xFC0000;
  uint32_t vMask = 0;

  uint8_t *base64Buf = (uint8_t *)malloc(base64BufLen);

  if (!base64Buf) {
    fprintf(stderr, "Failed to allocate memory for base64 string.\n");
    exit(1);
  }

  uint32_t threeBytes = 0, tmp = 0;
  size_t jCount = 0;
  uint32_t b64Chunk = 18;
  vMask = mask;
  // here we go
  for (size_t i = 0; i < binSize; i += 3) {
    if ((i + 3) > binSize && flag == 1) {
      tmp = binBuffer[i] << 4;
      base64Buf[jCount] = (tmp & 0xFC0) >> 6;
      base64Buf[jCount + 1] = tmp & 0x3F;
      break;
    } else if ((i + 3) > binSize && flag == 2) {
      tmp = (binBuffer[i] << 8) | (binBuffer[i + 1]);
      tmp = tmp << 2;
      base64Buf[jCount] = (tmp & 0x3F000) >> 12;
      base64Buf[jCount + 1] = (tmp & 0xFC0) >> 6;
      base64Buf[jCount + 2] = tmp & 0x3F;
      break;
    }
    threeBytes =
        (binBuffer[i] << 16) | (binBuffer[i + 1] << 8) | binBuffer[i + 2];
    for (size_t j = jCount; j < jCount + 4; ++j) {
      base64Buf[j] = (threeBytes & vMask) >> b64Chunk;
      vMask = vMask >> 6;
      b64Chunk = b64Chunk - 6;
    }
    jCount += 4;
    vMask = mask;
    b64Chunk = 18;
  }

  return base64Buf;
}

void pad_odd_bytes(char **b, size_t s) {
  *b = (char *)realloc(*b, s + 2);
  if (!*b) {
    fprintf(stderr, "unable to reallocate memory for binary buffer.\n");
    exit(1);
  }
  (*b)[s] = '0';
  (*b)[s + 1] = '\0';
}

int main() {

  const unsigned int MAX_BUFFER_SIZE = 256;
  const char base64_table[] =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  unsigned char *strBuffer = (unsigned char *)malloc(MAX_BUFFER_SIZE);
  uint8_t *b64Buffer;

  if (strBuffer == NULL) {
    fprintf(stderr, "unable to allocate memory for hex string.\n");
    return -1;
  }

  printf("Enter a hex stream: ");

  if (!fgets((char *)strBuffer, MAX_BUFFER_SIZE, stdin)) {
    fprintf(stderr, "unable to read input.\n");
    return -1;
  }

  trim_newline(strBuffer);
  size_t s = strlen((char *)strBuffer);

  if (s % 2) {
    pad_odd_bytes((char **)&strBuffer, s);
    s += 2;
  }

  size_t binSize = s >> 1;
  size_t base64BufLen = (((binSize + 2) / 3) << 2);
  uint8_t *binData = (uint8_t *)malloc(binSize);

  if (!binData) {
    fprintf(stderr, "unable to allocate memory for binary data");
  }
  uint8_t flag = 0;

  if (binSize % 3 == 1) {
    flag = 1;
    base64BufLen = base64BufLen - 2;
  } else if (binSize % 3 == 2) {
    flag = 2;
    base64BufLen = base64BufLen - 1;
  }

  // convert hex string to raw bin
  hex_str_to_bin((char *)strBuffer, binData, binSize);

  b64Buffer = hex_to_b64(binData, binSize, base64BufLen, flag);

  printf("base64 encoded: ");

  for (size_t i = 0; i < base64BufLen; i++)
    printf("%c", base64_table[b64Buffer[i]]);

  switch (flag) {
  case 1:
    printf("==");
    break;
  case 2:
    printf("=");
    break;
  }

  printf("\n");

  free(strBuffer);
  free(binData);
  free(b64Buffer);
  return 0;
}
