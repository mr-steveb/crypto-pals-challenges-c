#include <stdio.h>
#include <string.h>

// Function to convert a hex character to a decimal value
int hex_char_to_int(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
    if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
    return -1;
}

// Function to validate and possibly modify hex string length
void validate_and_pad_hex(char* hex, int max_len) {
    int len = strlen(hex);
    if (len % 2 != 0) {
        // Pad with '0' if the length is odd
        if (len < max_len - 1) {
            hex[len] = '0';
            hex[len + 1] = '\0';
        }
    }
}

// Function to convert a hex string to a byte array
int hex_to_binary(const char* hex, unsigned char* binary, int max_bytes) {
    int len = strlen(hex);
    for (int i = 0, j = 0; i < len; i += 2, j++) {
        int high = hex_char_to_int(hex[i]);
        int low = hex_char_to_int(hex[i + 1]);
        if (high == -1 || low == -1) return -1; // Invalid hex character

        if (j >= max_bytes) return -1; // Prevent buffer overflow
        binary[j] = (high << 4) + low;
    }
    return len / 2; // Return number of bytes in the binary array
}

// Base64 encoding table
static const char base64_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// Function to perform Base64 encoding
void base64_encode(unsigned char* bytes, int bytes_len, char* base64, int base64_size) {
    int i, j;
    for (i = 0, j = 0; i < bytes_len; i += 3) {
        int byte1 = bytes[i];
        int byte2 = i + 1 < bytes_len ? bytes[i + 1] : 0;
        int byte3 = i + 2 < bytes_len ? bytes[i + 2] : 0;

        int triple = (byte1 << 16) + (byte2 << 8) + byte3;

        if (j + 4 < base64_size) {
            base64[j++] = base64_table[(triple >> 18) & 0x3F];
            base64[j++] = base64_table[(triple >> 12) & 0x3F];
            base64[j++] = base64_table[(triple >> 6) & 0x3F];
            base64[j++] = base64_table[triple & 0x3F];
        }
    }

    // Padding
    if (bytes_len % 3 == 1) {
        base64[j - 2] = '=';
        base64[j - 1] = '=';
    } else if (bytes_len % 3 == 2) {
        base64[j - 1] = '=';
    }

    base64[j] = '\0'; // Null-terminate the string
}

int main() {
    char hex_input[256]; // Buffer for hex input (up to 256 bytes * 2 characters each + 1 for null-terminator)
    unsigned char binary[128];
    char base64[180]; // Sufficiently large buffer to hold base64 output

    printf("Enter a hex string (up to 256 characters): ");
    fgets(hex_input, sizeof(hex_input), stdin);
    hex_input[strcspn(hex_input, "\n")] = 0; // Remove newline character

    validate_and_pad_hex(hex_input, sizeof(hex_input));

    int binary_len = hex_to_binary(hex_input, binary, sizeof(binary));
    if (binary_len < 0) {
        fprintf(stderr, "Invalid hexadecimal input.\n");
        return 1;
    }

    base64_encode(binary, binary_len, base64, sizeof(base64));
    printf("Base64 Encoded: %s\n", base64);

    return 0;
}
