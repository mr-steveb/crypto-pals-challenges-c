#include <string.h>
#include <stdio.h>
#include <stdlib.h>
const static char key[] = "ICE";
/* const static char expected[] =
    "0b3637272a2b2e63622c2e69692a23693a2a3c6324202d623d63343c2a2622632427276527"
    "2a282b2f20430a652e2c652a3124333a653e2b2027630c692b20283165286326302e27282"
    "f"; */
size_t get_file_size(FILE **f) {
  fseek(*f, 0L, SEEK_END);
  int sz = ftell(*f);
  rewind(*f);
  printf("file size: %i\n",sz);
  return sz;
}

int main(int argc, char *argv[]) {

  FILE *fp = fopen(argv[1], "r");
  size_t fs = get_file_size(&fp);
  size_t key_index = 0;
  char *f = malloc(fs);
  fread(f, fs, 1, fp);
  fclose(fp);
  f[fs-1] = 0;

  for (size_t i = 0; i < fs; ++i) {
    printf("%02x", f[i] ^ key[key_index]);
    ++key_index;
    key_index %= strlen(key);
  }

  printf("\n");
  //printf("%s\n", expected);
  free(f);

  return 0;
}
