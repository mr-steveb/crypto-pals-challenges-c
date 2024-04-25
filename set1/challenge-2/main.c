#include <stdio.h>

const static unsigned char input[] =
    "\x1c\x01\x11\x00\x1f\x01\x01\x00\x06\x1a\x02\x4b"
    "\x53\x53\x50\x09\x18\x1c";
const static unsigned char xorKey[] =
    "\x68\x69\x74\x20\x74\x68\x65\x20\x62\x75\x6c\x6c"
    "\x27\x73\x20\x65\x79\x65";
const static char hex[] = "100111001f010100061a024b53535009181c";

unsigned char hex_char_to_byte(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    } else if (c >= 'a' && c <= 'f') {
        return 10 + c - 'a';
    } else if (c >= 'A' && c <= 'F') {
        return 10 + c - 'A';
    }
    return 0;
}

void decode_hex(const char *hex, unsigned char *decHex, size_t hexLen) {
    for (size_t i = 0, j = 0; i < hexLen; i += 2, j++) {
        decHex[j] = (hex_char_to_byte(hex[i]) << 4) | hex_char_to_byte(hex[i + 1]);
    }
}

void xor_buffers(const unsigned char *inputBuf, const unsigned char *xorKeyBuf, unsigned char *resBuf, size_t length) {
    for (size_t i = 0; i < length; ++i) {
        resBuf[i] = inputBuf[i] ^ xorKeyBuf[i];
    }
}

int main() {
    size_t hexLen = sizeof(hex) - 1; // Excluding the null terminator
    size_t decodedHexLen = hexLen / 2;
    size_t inputLen = sizeof(input) - 1; // Assuming consistent buffer sizes
    size_t xorKeyLen = sizeof(xorKey) - 1;
    size_t minLen = inputLen < xorKeyLen ? inputLen : xorKeyLen;

    unsigned char decodedHex[decodedHexLen];
    unsigned char output[minLen];

    decode_hex(hex, decodedHex, hexLen);
    xor_buffers(input, xorKey, output, minLen);

    printf("Decoded hex: ");
    for (size_t i = 0; i < decodedHexLen; i++) {
        printf("%02x", decodedHex[i]);
    }
    printf("\n");

    printf("Result: ");
    for (size_t i = 0; i < minLen; ++i) {
        printf("%02x", output[i]);
    }
    printf("\n");

    return 0;
}
