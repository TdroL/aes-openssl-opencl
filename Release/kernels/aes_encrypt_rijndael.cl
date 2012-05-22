#ifndef _AES_ENCRYPT_RIJNDAEL_CL
#define _AES_ENCRYPT_RIJNDAEL_CL

#define GETU32(plaintext) (((uint)(plaintext)[0] << 24) ^ \
						   ((uint)(plaintext)[1] << 16) ^ \
						   ((uint)(plaintext)[2] <<  8) ^ \
						   ((uint)(plaintext)[3]      ))

#define PUTU32(ciphertext, st) { (ciphertext)[0] = (uchar)((st) >> 24); \
								 (ciphertext)[1] = (uchar)((st) >> 16); \
								 (ciphertext)[2] = (uchar)((st) >>  8); \
								 (ciphertext)[3] = (uchar)((st)      ); }

#endif