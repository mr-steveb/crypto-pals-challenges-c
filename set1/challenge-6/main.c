#include <ctype.h>
#include <float.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *chars = "etaoin shrdlu";

int base64_char_to_val(char c) {
  if (c >= 'A' && c <= 'Z')
    return c - 'A';
  if (c >= 'a' && c <= 'z')
    return 26 + (c - 'a');
  if (c >= '0' && c <= '9')
    return 52 + (c - '0');
  if (c == '+')
    return 62;
  if (c == '/')
    return 63;
  return -1;
}

void base64_to_bin(const char *b64Str, uint8_t **blob, size_t length,
                   size_t *blob_size) {
  size_t index = 0;
  size_t outputIndex = 0;
  size_t padding = 0;

  // Count padding characters
  if (length >= 2) {
    if (b64Str[length - 1] == '=')
      padding++;
    if (b64Str[length - 2] == '=')
      padding++;
  }

  // Calculate expected binary size
  *blob_size = (length / 4) * 3 - padding;

  // Allocate memory for binary data
  *blob = (uint8_t *)malloc(*blob_size);
  if (*blob == NULL) {
    fprintf(stderr, "Unable to allocate memory for binary data.\n");
    exit(1);
  }

  uint32_t triple = 0;
  int bits_collected = 0;

  while (index < length && b64Str[index] != '=') {
    int value = base64_char_to_val(b64Str[index++]);
    if (value == -1)
      continue; // Skip invalid characters

    triple = (triple << 6) | value;
    bits_collected += 6;

    if (bits_collected >= 8) {
      bits_collected -= 8;
      (*blob)[outputIndex++] = (triple >> bits_collected) & 0xFF;
    }
  }

  *blob_size =
      outputIndex; // Adjust the size in case of any non-standard padding
}

// Efficient bit count using Brian Kernighan’s algorithm
int bit_count(size_t u) {
  int count = 0;
  while (u) {
    u &= (u - 1);
    count++;
  }
  return count;
}

// Calculate the Hamming distance between two chunks
size_t hamming_dist(const unsigned char *chunk1, const unsigned char *chunk2,
                    size_t len) {
  if (chunk1 == NULL || chunk2 == NULL) {
    fprintf(stderr, "NULL pointer supplied to hamming_dist\n");
    return 0;
  }
  size_t hamming_dist = 0;
  for (size_t i = 0; i < len; i++) {
    hamming_dist += bit_count(chunk1[i] ^ chunk2[i]);
  }
  return hamming_dist;
}

size_t find_best_key_size(unsigned char *data, size_t data_len) {
  size_t best_size = 0;
  size_t best_score = 1000000;
  int key = 0;

  for (key = 2; key <= 40; ++key) {
    int i = 0;
    size_t score = 0;
    for (i = 0; i < 10; ++i) {
      score += hamming_dist(&data[i * key], &data[(i + 1) * key], key);
    }
    score = score * 100 / key;
    if (score < best_score) {
      best_score = score;
      best_size = key;
    }
  }
  return best_size;
}

char *load_file_into_memory(const char *filename, size_t *length) {

  FILE *file = fopen(filename, "rb"); // Open the file in binary mode

  if (!file) {
    fprintf(stderr, "Unable to open file %s\n", filename);
    return NULL;
  }

  // Seek to the end of the file to determine the file size
  fseek(file, 0, SEEK_END);
  size_t size = ftell(file);

  if (size == 0) {
    fprintf(stderr, "File is empty: %s\n", filename);
    fclose(file);
    return NULL;
  }

  rewind(file); // Go back to the start of the file

  // Allocate memory for the entire file
  char *data = malloc(size + 1);
  if (!data) {
    fclose(file);
    fprintf(stderr, "Memory allocation failed\n");
    return NULL;
  }

  // Read the file into memory and null-terminate it
  size_t bytes_read = fread(data, 1, size, file);

  if (bytes_read < size) {
    free(data);
    fclose(file);
    fprintf(stderr, "Failed to read the whole file\n");
    return NULL;
  }

  data[size] = '\0'; // Null-terminate the data
  *length = size;    // Set the output length
  fclose(file);      // Close the filename

  size_t offset = 0;

  for (size_t i = 0; i < *length; i++) {
    if (data[i] != '\n' && data[i] != '\r') {
      data[offset++] = data[i];
    }
  }

  data[offset] = '\0'; // Re-null-terminate the string
  *length = offset;
  return data; // Return the file data
}

int char_score(char c) {

  int score = 0;
  c = tolower(c);

  for (int i = strlen(chars) -1; i >= 0; --i) {
    if (chars[i] == c) {
      score = 12 - i;
      break;
    }
  }
  return score;
}


void ascertain_key(unsigned char *data, size_t data_len, unsigned char *key,
                   size_t key_len) {

  for (size_t i = 0; i < key_len; ++i) {
    // xor byte against
    unsigned char best_byte = 0;
    int best_score = 0;
    int byte = 0;
    for (byte = 0; byte <= 255; byte++) {
      size_t j = 0;
      int score = 0;
      for (j = i; j < data_len; j += key_len) {
        score += char_score(data[j] ^ (unsigned char)byte);
      }

      if (score > best_score) {
        best_score = score;
        best_byte = byte;
      }
    }
    key[i] = best_byte;
  }
}

void repeating_key_decrypt(unsigned char *data, unsigned char *key,
                           size_t data_len, size_t key_len) {

  for (size_t i = 0, j = 0; i < data_len; ++i) {
    printf("%c", data[i] ^ key[j]);
    ++j;
    j %= key_len;
  }
}

int main(int argc, char *argv[]) {

  if (argc < 2) {
    fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
    return EXIT_FAILURE;
  }

  size_t length = 0;
  size_t output_length = 0;
  char *eb64p = load_file_into_memory(argv[1], &length);
  unsigned char *blob;
  unsigned char *key;

  if (!eb64p) {
    fprintf(stderr, "Error loading file into memory.\n");
    return EXIT_FAILURE;
  }

  // unsigned char *blob = base64_decode(eb64p, length, &output_length);
  base64_to_bin(eb64p, &blob, length, &output_length);

  if (!blob) {
    fprintf(stderr, "Base64 decoding failed.\n");
    free(eb64p); // Clean up previously allocated memory
    return EXIT_FAILURE;
  }

  size_t best_key_length = find_best_key_size(blob, output_length);

  key = malloc(best_key_length);

  ascertain_key(blob, output_length, key, best_key_length);

  repeating_key_decrypt(blob, key, output_length, best_key_length);

  free(eb64p);
  free(blob);
  free(key);
  return 0;
}
