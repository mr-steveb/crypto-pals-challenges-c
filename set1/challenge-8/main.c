#include <float.h>
#include <openssl/aes.h>
#include <openssl/conf.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  char *content;
  size_t length;
} Line;

typedef struct {
  Line *lines;
  size_t count;
} FileContent;

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
      exit(-1);
    }
  }
}

FileContent *load_file_by_lines(const char *filename) {

  FILE *file = fopen(filename, "r");

  if (!file) {
    fprintf(stderr, "Unable to open file %s\n", filename);
    return NULL;
  }

  FileContent *file_content = malloc(sizeof(FileContent));
  if (!file_content) {
    fclose(file);
    fprintf(stderr, "Memory allocation failed for FileContent\n");
    return NULL;
  }

  file_content->count = 0;
  file_content->lines = NULL;

  char *line_buffer = NULL;
  size_t line_buffer_size = 0;
  ssize_t line_size;
  size_t line_count = 0;

  while ((line_size = getline(&line_buffer, &line_buffer_size, file)) != -1) {
    if (line_size > 0 && line_buffer[line_size - 1] == '\n') {
      line_buffer[line_size - 1] = '\0'; // Remove newline character
      line_size--;
    }

    Line line;
    line.content = strndup(line_buffer, line_size);
    line.length = line_size;

    Line *new_lines =
        realloc(file_content->lines, (line_count + 1) * sizeof(Line));
    if (!new_lines) {
      // Handle error: free resources
      free(line_buffer);
      for (size_t i = 0; i < line_count; i++) {
        free(file_content->lines[i].content);
      }
      free(file_content->lines);
      free(file_content);
      fclose(file);
      return NULL;
    }

    file_content->lines = new_lines;
    file_content->lines[line_count] = line;
    line_count++;
  }
  free(line_buffer);
  file_content->count = line_count;
  fclose(file);

  return file_content;
}

void free_file_content(FileContent *fc) {
  for (size_t i = 0; i < fc->count; i++) {
    free(fc->lines[i].content);
  }
  free(fc->lines);
  free(fc);
}

int find_ecb_block(FileContent *fc) {

  int best_score = -1, best_line = -1;

  unsigned char *raw_hex;
  size_t raw_hex_len;

  for (size_t i = 0; i < fc->count; ++i) {

    int score = 0;
    raw_hex_len = fc->lines[i].length / 2;
    raw_hex = (unsigned char *)malloc(raw_hex_len);

    if (!raw_hex) {
      fprintf(stderr, "unable to allocate memory.\n");
      return -1;
    }

    hex_str_to_bin(fc->lines[i].content, raw_hex, raw_hex_len);

    for (size_t j = 0; j < raw_hex_len; j += 16) {
      for (size_t k = j + 16; k < raw_hex_len; k += 16)
        if (0 == memcmp(&raw_hex[j], &raw_hex[k], 16))
          ++score;
    }

    if (score > best_score) {
      best_score = score;
      best_line = i;
    }
    free(raw_hex);
  }

  return best_line;
}

int main(int argc, char *argv[]) {

  if (argc < 2) {
    fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
    exit(-1);
  }

  size_t len = 0, out_buff_size = 0, out_len = 0;
  FileContent *fc = load_file_by_lines(argv[1]);

  if (!fc) {
    fprintf(stderr, "Error loading file into memory.\n");
    return EXIT_FAILURE;
  }

  int best_line = find_ecb_block(fc);

  printf("best block is no. %i\n", best_line);

  for (size_t i = 0; i < fc->lines[best_line].length; ++i) {
    if (!(i & 31) && i != 0) // sorry :(
      printf("\n");

    printf("%c", fc->lines[best_line].content[i]);
  }
  printf("\n");
  free_file_content(fc);

  return EXIT_SUCCESS;
}
