#include "stdafx.h"
#include "cl_encrypt.h"
#include "cl_encrypt_test.h"
#include "rijndael.h"

#define KEYBITS 256
#define DATA_LENGTH 128

struct Encrypt_Data {
	unsigned int length;
	char kernel_name[32];
	uint8_t key[32];
	uint8_t plaintext[DATA_LENGTH];
	uint8_t ciphertext[DATA_LENGTH];
};

static Encrypt_Data test_data[] = {
	#include "cl_encrypt_test_data.txt"
	{0}
};

void local_encrypt(uint8_t *plaintext, uint32_t *rk);

void cl_encrypt_test()
{
	printf("Test: cl_encrypt\n");

	//uint8_t result[DATA_LENGTH];

	for (unsigned int i = 0; test_data[i].length; i++)
	{
		Encrypt_Data &data = test_data[i];

		if ( ! cl_init(data.kernel_name, DATA_LENGTH, RKLENGTH(data.length)*sizeof(uint32_t), true))
		{
			printf("  #%02u (key %u bits, kernel %s)  skip\n", i+1, data.length, data.kernel_name);
			continue;
		}

		uint32_t rk[RKLENGTH(256)];
		uint8_t plaintext[DATA_LENGTH];

		memcpy(plaintext, data.plaintext, DATA_LENGTH);

		rijndaelSetupEncrypt((unsigned long *) rk, data.key, data.length);

		//memcpy(result, data.plaintext, DATA_LENGTH);

		cl_update(data.plaintext, rk);
		//local_encrypt(plaintext, rk);

		if (memcmp(data.plaintext, data.ciphertext, DATA_LENGTH) == 0)
		{
			printf("  #%02u (key %u bits, kernel %s)  pass\n", i+1, data.length, data.kernel_name);
		}
		else
		{
			printf("  #%02u (key %u bits, kernel %s)  fail\n", i+1, data.length, data.kernel_name);
			/*
			for (unsigned int j = 0; j < 16; j += 4)
			{
				printf("    ");
				printf("0x%08x  ", ((uint_u *) (data.plaintext + j))->i);
				printf("0x%08x  ", ((uint_u *) (data.ciphertext + j))->i);
				printf("%2i\n", memcmp(data.plaintext + j, data.ciphertext + j, 4));
			}
			*/
		}
	}

	cl_release_all();
}