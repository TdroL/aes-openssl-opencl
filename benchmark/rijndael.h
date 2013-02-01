#ifndef H__RIJNDAEL
#define H__RIJNDAEL

int rijndaelSetupEncrypt(unsigned long *rk, const unsigned char *key,
  int keybits);
int rijndaelSetupDecrypt(unsigned long *rk, const unsigned char *key,
  int keybits);
void rijndaelEncrypt(const unsigned long *rk, int nrounds,
  const unsigned char plaintext[16], unsigned char ciphertext[16]);
void rijndaelDecrypt(const unsigned long *rk, int nrounds,
  const unsigned char ciphertext[16], unsigned char plaintext[16]);

#ifndef KEYLENGTH
	#define KEYLENGTH(keybits) ((keybits)/8)
#endif
#ifndef RKLENGTH
	#define RKLENGTH(keybits)  ((keybits)/8+28)
#endif
#ifndef NROUNDS
	#define NROUNDS(keybits)   ((keybits)/32+6)
#endif

#endif