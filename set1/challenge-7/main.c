#include <float.h>
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/aes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const unsigned char key[] = "YELLOW SUBMARINE";

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

void handleErrors(void)
{
    ERR_print_errors_fp(stderr);
    abort();
}

unsigned char *load_file_into_memory(const char *filename, size_t *length) {

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
  unsigned char *data = malloc(size + 1);
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

int decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *key,
            unsigned char *iv, unsigned char *plaintext)
{
    EVP_CIPHER_CTX *ctx;

    int len;

    int plaintext_len;

    /* Create and initialise the context */
    if(!(ctx = EVP_CIPHER_CTX_new()))
        handleErrors();

    /*
     * Initialise the decryption operation. IMPORTANT - ensure you use a key
     * and IV size appropriate for your cipher
     * In this example we are using 256 bit AES (i.e. a 256 bit key). The
     * IV size for *most* modes is the same as the block size. For AES this
     * is 128 bits
     */

    if(1 != EVP_DecryptInit_ex(ctx, EVP_aes_128_ecb(), NULL, key, iv))
        handleErrors();

    /*
     * Provide the message to be decrypted, and obtain the plaintext output.
     * EVP_DecryptUpdate can be called multiple times if necessary.
     */
    if(1 != EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len))
        handleErrors();
    plaintext_len = len;

    /*
     * Finalise the decryption. Further plaintext bytes may be written at
     * this stage.
     */
    if(1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len))
        handleErrors();
    plaintext_len += len;

    /* Clean up */
    EVP_CIPHER_CTX_free(ctx);

    return plaintext_len;
}

int main(int argc, char *argv[]) {

  if (argc < 2) {
    fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
    exit(-1);
  }

  size_t len = 0, out_buff_size = 0, out_len = 0;
  char *b64_mem = (char *)load_file_into_memory(argv[1], &len);
  unsigned char *blob;

  if (!b64_mem) {
    fprintf(stderr, "Error loading file into memory.\n");
    return EXIT_FAILURE;
  }

  base64_to_bin(b64_mem, &blob, len, &out_buff_size);

  if (!blob) {
    fprintf(stderr, "Base64 decoding failed.\n");
    free(b64_mem); // Clean up previously allocated memory
    return EXIT_FAILURE;
  }
  
  unsigned char out[out_buff_size];

  out_len = (size_t)decrypt(blob, out_buff_size, (unsigned char*)key, NULL, out);

  out[out_len] = '\0';

  printf("decrypted text: %s\n",out);

  free(b64_mem);
  free(blob);

  return 1;
}
