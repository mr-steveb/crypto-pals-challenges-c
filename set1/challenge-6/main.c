#include <float.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char DECODING_TABLE[] = {
    62, -1, -1, -1, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1,
    -1, -1, -1, -1, -1, -1, 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
    10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
    -1, -1, -1, -1, -1, -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35,
    36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51};

typedef struct {
  size_t keySize;
  float normalizedDistance;
} KEYSIZE_RESULT;

int compare_keysize_result(const void *a, const void *b) {
  KEYSIZE_RESULT *resultA = (KEYSIZE_RESULT *)a;
  KEYSIZE_RESULT *resultB = (KEYSIZE_RESULT *)b;
  return (resultA->normalizedDistance > resultB->normalizedDistance) -
         (resultA->normalizedDistance < resultB->normalizedDistance);
}

// Function to decode a Base64 encoded string
unsigned char *base64_decode(const char *data, size_t input_length,
                             size_t *output_length) {
  if (input_length % 4 != 0) {
    fprintf(stderr, "Invalid Base64 length: must be multiple of 4\n");
    return NULL;
  }

  *output_length = (input_length / 4) * 3;
  if (data[input_length - 1] == '=')
    (*output_length)--;
  if (data[input_length - 2] == '=')
    (*output_length)--;

  unsigned char *decoded_data = (unsigned char *)malloc(*output_length);

  if (decoded_data == NULL) {
    fprintf(stderr, "Failed to allocate memory for decoded data\n");
    return NULL;
  }

  for (int i = 0, j = 0; i < input_length;) {
    uint32_t sextet_a = 0, sextet_b = 0, sextet_c = 0, sextet_d = 0;

    // Process first sextet
    int idx_a = data[i] - 43;
    if (data[i] == '=' || idx_a < 0 || idx_a >= sizeof(DECODING_TABLE) ||
        DECODING_TABLE[idx_a] == -1) {
      fprintf(stderr, "Invalid Base64 character: %c\n", data[i]);
      free(decoded_data);
      return NULL;
    }
    sextet_a = DECODING_TABLE[idx_a];
    ++i;

    // Process second sextet
    int idx_b = data[i] - 43;
    if (data[i] == '=' || idx_b < 0 || idx_b >= sizeof(DECODING_TABLE) ||
        DECODING_TABLE[idx_b] == -1) {
      fprintf(stderr, "Invalid Base64 character: %c\n", data[i]);
      free(decoded_data);
      return NULL;
    }
    sextet_b = DECODING_TABLE[idx_b];
    ++i;

    // Process third sextet
    int idx_c = data[i] - 43;
    if (data[i] == '=' || idx_c < 0 || idx_c >= sizeof(DECODING_TABLE) ||
        DECODING_TABLE[idx_c] == -1) {
      if (data[i] != '=' || i + 1 != input_length ||
          data[i + 1] != '=') { // Allow '=' only if it's padding at the end
        fprintf(stderr, "Invalid Base64 character: %c\n", data[i]);
        free(decoded_data);
        return NULL;
      }
      sextet_c = 0; // Padding character '='
    } else {
      sextet_c = DECODING_TABLE[idx_c];
    }
    ++i;

    // Process fourth sextet
    int idx_d = data[i] - 43;
    if (data[i] == '=' || idx_d < 0 || idx_d >= sizeof(DECODING_TABLE) ||
        DECODING_TABLE[idx_d] == -1) {
      if (data[i] != '=' ||
          i != input_length -
                   1) { // Allow '=' only if it's the very last character
        fprintf(stderr, "Invalid Base64 character: %c\n", data[i]);
        free(decoded_data);
        return NULL;
      }
      sextet_d = 0; // Padding character '='
    } else {
      sextet_d = DECODING_TABLE[idx_d];
    }
    ++i;

    // Combine the four sextets into three bytes
    uint32_t triple =
        (sextet_a << 18) + (sextet_b << 12) + (sextet_c << 6) + sextet_d;

    if (j < *output_length)
      decoded_data[j++] = (triple >> 16) & 0xFF;
    if (j < *output_length && data[i - 2] != '=')
      decoded_data[j++] = (triple >> 8) & 0xFF;
    if (j < *output_length && data[i - 1] != '=')
      decoded_data[j++] = triple & 0xFF;
  }

  return decoded_data;
}

int bit_count(size_t u) {
  size_t uCount;

  uCount = u - ((u >> 1) & 033333333333) - ((u >> 2) & 011111111111);
  return ((uCount + (uCount >> 3)) & 030707070707) % 63;
}

size_t hamming_diff(unsigned char *str1, unsigned char *str2, size_t len) {

  if (str1 == NULL || str2 == NULL) {
    fprintf(stderr, "NULL pointer supplied to hamming_diff\n");
    return 0;
  }
  unsigned char diff;
  size_t hDis = 0;
  for (size_t i = 0; i < len; i++) {
    diff = str1[i] ^ str2[i];
    hDis += bit_count(diff); // Count bits per byte immediately
  }
  return hDis;
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

int main(int argc, char *argv[]) {

  if (argc < 2) {
    fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
    return EXIT_FAILURE;
  }

  size_t length = 0;
  size_t output_length = 0;
  char *eb64p = load_file_into_memory(argv[1], &length);
  if (!eb64p) {
    fprintf(stderr, "Error loading file into memory.\n");
    return EXIT_FAILURE;
  }

  unsigned char *db64p = base64_decode(eb64p, length, &output_length);

  if (!db64p) {
    fprintf(stderr, "Base64 decoding failed.\n");
    free(eb64p); // Clean up previously allocated memory
    return EXIT_FAILURE;
  }

  printf("file length: %lu\n", length);
  printf("decoded length: %lu\n", output_length);

  int block_pairs = 4;        // Number of block pairs to average
  KEYSIZE_RESULT results[40]; // Assuming the maximum KEYSIZE is 40

  for (int i = 0; i < 40; ++i) {
    results[i].keySize = i + 2; // Adjust according to actual key size range
    results[i].normalizedDistance =
        FLT_MAX; // Use float.h to set initial high values
  }

  for (size_t key_length = 2; key_length <= 40; ++key_length) {
    float total_normalized_distance = 0;
    int valid_pairs = 0;

    for (int pair = 0;
         pair < block_pairs && (pair + 1) * key_length * 2 <= output_length;
         ++pair) {
      unsigned char *buff1 = db64p + (2 * pair * key_length);
      unsigned char *buff2 = buff1 + key_length;
      if (buff2 + key_length >
          db64p + output_length) { // Make sure buff2 does not go out of bounds
        break;
      }
      size_t hDis = hamming_diff(buff1, buff2, key_length);
      float normalized_hDis = (float)hDis / key_length;
      total_normalized_distance += normalized_hDis;
      valid_pairs++;
    }

    if (valid_pairs > 0) {
      float average_normalized_distance =
          total_normalized_distance / valid_pairs;
      if (key_length <
          40) { // Ensure we do not exceed the bounds of the results array
        results[key_length].keySize = key_length;
        results[key_length].normalizedDistance = average_normalized_distance;
      }
    }
  }

  qsort(results, 40, sizeof(KEYSIZE_RESULT), compare_keysize_result);

  size_t best_key_length = results[0].keySize;

  unsigned char **blocks;

  size_t num_blocks =
      output_length / best_key_length + (output_length % best_key_length != 0);
  blocks = malloc(num_blocks * sizeof(unsigned char *));

  for (size_t i = 0; i < num_blocks; ++i) {
    blocks[i] = malloc(best_key_length * sizeof(unsigned char));
    size_t block_size = (i * best_key_length + best_key_length <= output_length)
                            ? best_key_length
                            : output_length % best_key_length;
    memcpy(blocks[i], db64p + i * best_key_length, block_size);
  }

  free(eb64p);
  free(db64p);
  return 0;
}
