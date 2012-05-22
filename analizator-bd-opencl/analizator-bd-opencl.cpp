#include "stdafx.h"
#include "CL/cl.h"
#include "cl_encrypt.h"
#ifdef TEST_CL_ENCRYPT
	#include "cl_encrypt_test.h"
#endif
#include <fstream>
#include <sstream>
#include <ctime>
#include "rijndael.h"

#define KiB * 1024
#define MiB * 1024 KiB

int main(int argc, char **argv)
{
#ifdef TEST_CL_ENCRYPT
	cl_encrypt_test();
#endif

	unsigned int size_of_data = 1 MiB;
	unsigned int loops = 256;
	char *kernel_name = "aes_encrypt_rijndael_256";
	
	// analizator aes_encrypt_vector_256
	if (argc > 1)
	{
		kernel_name = argv[1];
	}

	// analizator [...] 1024 # 1 KiB
	if (argc > 2)
	{
		size_of_data = atoi(argv[2]);
	}

	// analizator [...] 16 M # 16 * MiB
	if (argc > 3)
	{
		switch (argv[3][0])
		{
		case 'm':
		case 'M':
			size_of_data *= 1 MiB;
		break;
		case 'k':
		case 'K':
			size_of_data *= 1 KiB;
		break;
		}
	}

	// analizator [...] 1 K 1024 # 1 KiB, 1024 loops
	if (argc > 4)
	{
		loops = std::atoi(argv[4]);
	}

	const size_t rk_length = 256;
	uint32_t rk[RKLENGTH(rk_length)];
	char key[KEYLENGTH(rk_length)] = "tajny klucz";
	uint8_t *data = new uint8_t[size_of_data];

	rijndaelSetupEncrypt((unsigned long *) rk, (uint8_t *) key, rk_length);

	cl_init(kernel_name, size_of_data * sizeof(*data), RKLENGTH(rk_length) * sizeof(*rk));

#ifdef PRINT_PLATFORM_INFO
	cl_print_platform_info();
#endif

#ifndef PRINT_DEVICE_INFO
	cl_print_device_info();
	exit(EXIT_SUCCESS);
#endif

	// find any errors in first run
	cl_update(data, rk);

	clock_t start, end;
	std::stringstream results;
	std::fstream f;

	results << "./results-bd-opencl/" << kernel_name << "-";

	if (size_of_data >= 1 MiB)
	{
		printf("Seria: %iMiB\n", (size_of_data / (1 MiB)));
		results << (size_of_data / (1 MiB)) << "MiB.txt";
	}
	else if (size_of_data >= 1 KiB)
	{
		printf("Seria: %iKiB\n", size_of_data);
		results << size_of_data << "KiB.txt";
	}
	else
	{
		printf("Seria: %iB\n", size_of_data);
		results << size_of_data << "B.txt";
	}

	f.open(results.str(), std::fstream::out | std::fstream::trunc);

	if ( ! f.is_open())
	{
		printf("Nie mozna otworzyc %s\n", results.str().c_str());
		exit(EXIT_FAILURE);
	}

	for (unsigned int i = 0; i < loops; i++)
	{
		for (size_t j = 0; j < size_of_data; data[j] = j+i, j++);

		start = clock();
		cl_update(data, rk);
		end = clock();

		printf("\r%i/%i", i+1, loops);

		f << (end - start) << "\n";
		f.flush();

		// fake sleep
		while ((clock() - end) < (50 * CLOCKS_PER_SEC / 1000));
	}

	printf("\r                                \r");
	f.close();
	
	/*
	printf("  ");
	for (size_t pos = 0; pos < 8; pos++)
	{
		printf("%08x ", *((uint32_t *) &data[pos*4]));
	}
	printf("\n");
	*/
	/*
	printf("OpenCL   %ums\n", ((1000*(end-start))/CLOCKS_PER_SEC));

	memset(data, 0, data_length);
	for (size_t i = 0; i < data_length; data[i] = i, i++);

	start = clock();
	for (size_t pos = 0; pos < data_length; pos += 16)
	{
		rijndaelEncrypt((unsigned long *) rk, 14, data + pos, data + pos);
	}
	end = clock();
	*/
	/*
	printf("  ");
	for (size_t pos = 0; pos < 8; pos++)
	{
		printf("%08x ", *((uint32_t *) &data[pos*4]));
	}
	printf("\n");
	*/
	/*
	printf("Rijndael %ums\n", ((1000*(end-start))/CLOCKS_PER_SEC));
	*/
	/*
	for (size_t pos = 0; pos < data_length; pos += 512*16)
	{
		uint8_t zeros[512*8] = {0};

		if (memcmp(data + pos, zeros, 512*8) != 0)
		{
			printf("%u\n", pos);
		}
	}
	*/

	cl_release_all();
	
	delete[] data;

	return EXIT_SUCCESS;
}
