#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char base64_table[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

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
        uint8_t highNibble = hex_char_to_bin(hexStr[2 * i]);
        uint8_t lowNibble = hex_char_to_bin(hexStr[2 * i + 1]);
        if (highNibble >= 0 && lowNibble >= 0) {
            binData[i] = (highNibble << 4) | lowNibble;
        } else {
            fprintf(stderr, "Invalid Hex character.\n");
            exit(1);
        }
    }
}

uint8_t *hex_to_b64(const uint8_t *binBuffer, size_t binSize, size_t *base64BufLen) {
    size_t outputLength = 4 * ((binSize + 2) / 3); // Calculate Base64 output buffer size
    if (base64BufLen) {
        *base64BufLen = outputLength;
    }

    uint8_t *base64Buf = (uint8_t *)malloc(outputLength + 1); // +1 for null-terminator
    if (!base64Buf) {
        fprintf(stderr, "Failed to allocate memory for base64 string.\n");
        exit(1);
    }

    size_t j = 0;
    for (size_t i = 0; i < binSize; i += 3) {
        uint32_t threeBytes = binBuffer[i] << 16;
        if (i + 1 < binSize) {
            threeBytes |= binBuffer[i + 1] << 8;
        }
        if (i + 2 < binSize) {
            threeBytes |= binBuffer[i + 2];
        }

        base64Buf[j++] = base64_table[(threeBytes >> 18) & 0x3F];
        base64Buf[j++] = base64_table[(threeBytes >> 12) & 0x3F];
        if (i + 1 < binSize) {
            base64Buf[j++] = base64_table[(threeBytes >> 6) & 0x3F];
        } else {
            base64Buf[j++] = '=';
        }
        if (i + 2 < binSize) {
            base64Buf[j++] = base64_table[threeBytes & 0x3F];
        } else {
            base64Buf[j++] = '=';
        }
    }

    base64Buf[j] = '\0'; // Null-terminate the output string
    return base64Buf;
}

int main() {
    const unsigned int MAX_BUFFER_SIZE = 256;
    unsigned char *strBuffer = (unsigned char *)malloc(MAX_BUFFER_SIZE);

    if (strBuffer == NULL) {
        fprintf(stderr, "Unable to allocate memory for hex string.\n");
        return -1;
    }

    printf("Enter a hex stream: ");
    if (!fgets((char *)strBuffer, MAX_BUFFER_SIZE, stdin)) {
        fprintf(stderr, "Unable to read input.\n");
        return -1;
    }

    // Remove newline character from input
    strBuffer[strcspn((char *)strBuffer, "\n")] = '\0';

    size_t hexLen = strlen((char *)strBuffer);
    if (hexLen % 2 != 0) {
        fprintf(stderr, "Hex string must have an even length.\n");
        free(strBuffer);
        return -1;
    }

    size_t binSize = hexLen / 2;
    uint8_t *binData = (uint8_t *)malloc(binSize);
    if (!binData) {
        fprintf(stderr, "Unable to allocate memory for binary data.\n");
        free(strBuffer);
        return -1;
    }

    hex_str_to_bin((char *)strBuffer, binData, binSize);

    size_t base64BufLen;
    uint8_t *b64Buffer = hex_to_b64(binData, binSize, &base64BufLen);

    printf("Base64 encoded: %s\n", b64Buffer);

    free(strBuffer);
    free(binData);
    free(b64Buffer);
    return 0;
}
