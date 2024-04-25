#include <stdio.h>

const static unsigned char input[] =
    "\x1c\x01\x11\x00\x1f\x01\x01\x00\x06\x1a\x02\x4b"
    "\x53\x53\x50\x09\x18\x1c";
const static unsigned char xorKey[] =
    "\x68\x69\x74\x20\x74\x68\x65\x20\x62\x75\x6c\x6c"
    "\x27\x73\x20\x65\x79\x65";
const static unsigned char hex[] = "100111001f010100061a024b53535009181c";

unsigned char hex_chars_to_byte(char c) {

  return (c >= '0' && c <= '9')   ? c - '0'
         : (c >= 'a' && c <= 'f') ? 10 + (c - 'a')
         : (c >= 'A' && c <= 'F') ? 10 + (c - 'A')
                                  : 0;
}

void decode_hex(const unsigned char *hex, unsigned char *decHex, size_t hexLen,
                size_t decHexLen) {

  for (size_t i = 0, j = 0; i < hexLen; i += 2)
    decHex[j++] =
        (hex_chars_to_byte(hex[i]) << 4) | hex_chars_to_byte(hex[i + 1]);
}

void xor_buffers(const unsigned char *inputBuf, const unsigned char *xorKeyBuf,
                 unsigned char *resBuf, size_t length) {

  for (size_t i = 0; i < length; ++i) {
    resBuf[i] = inputBuf[i] ^ xorKeyBuf[i];
  }
}

int main() {

  size_t hexLen = sizeof(hex);
  size_t decHexLen = hexLen >> 1;
  size_t inputLen = sizeof(input);
  size_t outputLen = inputLen;
  unsigned char decodedHex[decHexLen + 1];
  unsigned char output[outputLen + 1];
  output[outputLen] = '\0';
  decodedHex[decHexLen] = '\0';

  decode_hex(hex, decodedHex, hexLen, decHexLen);
  xor_buffers(input, xorKey, output, inputLen);

  printf("Decoded hex :");
  for (size_t i = 0; i < decHexLen; i++)
    printf("%02x", decodedHex[i]);

  printf("\n");

  printf("Result: ");
  for (size_t i = 0; i < outputLen - 1; ++i)
    printf("%02x", output[i]);

  printf("\n");

  return 0;
}
