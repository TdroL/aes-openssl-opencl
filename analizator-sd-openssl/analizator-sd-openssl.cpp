#include "stdafx.h"
#include <fstream>
#include <sstream>
#include <cstdio>
#include <limits>
#include <ctime>
#include <cstdint>
#include <openssl/evp.h>

#define KiB 1024
#define MiB (1024*KiB)

int main (int argc, char **argv) {

	unsigned int size_of_data = 1 * KiB;
	unsigned int outer_loops = 256;
	unsigned int inner_loops = 1024;

	// analizator 1024 # 1 KiB
	if (argc > 1)
	{
		size_of_data = atoi(argv[1]);
	}

	// analizator 16 K # 16 * KiB
	if (argc > 2)
	{
		switch (argv[2][0])
		{
		case 'm':
		case 'M':
			size_of_data *= MiB;
		break;
		case 'k':
		case 'K':
			size_of_data *= KiB;
		break;
		}
	}

	// analizator 1 K 1024 # 1 KiB, 1024 outer loops
	if (argc > 3)
	{
		outer_loops = atoi(argv[3]);
	}
	
	// analizator 1 K 1024 1000 # 1 KiB, 1000 outer loops, 1000 inner loops
	if (argc > 4)
	{
		inner_loops = atoi(argv[4]);
	}

	std::stringstream results;
	std::fstream f;

	results << "./results-sd-openssl/";

	if (size_of_data >= MiB)
	{
		printf("Seria: %iMiB, %i loops\n", (size_of_data / MiB), inner_loops);
		results << (size_of_data / MiB) << "MiB-(" << inner_loops << ").txt";
	}
	else if (size_of_data >= KiB)
	{
		printf("Seria: %iKiB, %i loops\n", (size_of_data / KiB), inner_loops);
		results  << (size_of_data / KiB) << "KiB-(" << inner_loops << ").txt";
	}
	else
	{
		printf("Seria: %iB, %i loops\n", size_of_data, inner_loops);
		results  << size_of_data << "B-(" << inner_loops << ").txt";
	}

	f.open(results.str(), std::fstream::out | std::fstream::trunc);

	if ( ! f.is_open())
	{
		printf("Nie mozna otworzyc %s\n", results.str().c_str());
		exit(EXIT_FAILURE);
	}
	
	for (unsigned int i = 0; i < outer_loops; i++)
	{
		unsigned char *inbuf = new unsigned char[size_of_data * inner_loops];
		unsigned char *outbuf = new unsigned char[size_of_data * inner_loops];
		int inlen = size_of_data,
			outlen = 0;

		// fill input buffer
		memset(inbuf, i, sizeof(unsigned char) * size_of_data * inner_loops);

		// simple key and iv
		unsigned char key[] = {	0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 };
		unsigned char iv[] = { 1,2,3,4,5,6,7,8 };

		clock_t start, end;

		// init encrypt context
		EVP_CIPHER_CTX ctx;
		EVP_CIPHER_CTX_init(&ctx);
		EVP_EncryptInit_ex(&ctx, EVP_aes_256_ecb(), NULL, key, iv);

		int status = 0;

		start = clock();
		for (unsigned int j = 0; j < inner_loops; j++)
		{
			// encrypt
			status |= EVP_EncryptUpdate(&ctx, outbuf, &outlen, inbuf + (j * size_of_data), inlen);
		}
		end = clock();

		if ( ! status)
		{
			printf("Error: EVP_EncryptUpdate\n");
			return 0;
		}
			
		delete[] inbuf;
		delete[] outbuf;
		EVP_CIPHER_CTX_cleanup(&ctx);

		printf("\r%i/%i", i+1, outer_loops);

		f << (end - start) << "\n";

		// fake sleep
		while ((clock() - end) < (10 * CLOCKS_PER_SEC / 1000));
	}

	printf("\r                                \r");
	f.close();

	//system("pause");
	return EXIT_SUCCESS;
}