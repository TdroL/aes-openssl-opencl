#include <stdio.h>
#include "rijndael.h"

#define KEYBITS 256

int main(int argc, char **argv)
{
  unsigned long rk[RKLENGTH(KEYBITS)];
  unsigned char key[KEYLENGTH(KEYBITS)];
  int i;
  int nrounds;
  char *password;
  FILE *input;
  if (argc < 3)
  {
    fputs("Missing argument", stderr);
    return 1;
  }
  password = argv[1];
  for (i = 0; i < sizeof(key); i++)
    key[i] = *password != 0 ? *password++ : 0;
  input = fopen(argv[2], "rb");
  if (input == NULL)
  {
    fputs("File read error", stderr);
    return 1;
  }
  nrounds = rijndaelSetupDecrypt(rk, key, 256);
  while (1)
  {
    unsigned char plaintext[16];
    unsigned char ciphertext[16];
    int j;
    if (fread(ciphertext, sizeof(ciphertext), 1, input) != 1)
      break;
    rijndaelDecrypt(rk, nrounds, ciphertext, plaintext);
    fwrite(plaintext, sizeof(plaintext), 1, stdout);
  }
  fclose(input);
}