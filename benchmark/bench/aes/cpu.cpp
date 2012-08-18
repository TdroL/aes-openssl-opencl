#include "stdafx.h"
#include "cpu.h"
#include "boost/chrono.hpp"

#ifdef _DEBUG
#include <iostream>
#endif

namespace Bench
{
namespace Aes
{

using namespace std;
namespace ch = boost::chrono;
typedef ch::high_resolution_clock hrc;


bool Cpu::init(size_t sampleLength, size_t keyLength)
{
	unsigned char key[] = { 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16 };

	EVP_CIPHER_CTX_init(&ctx);

	switch (keyLength)
	{
	case 128:
		return EVP_EncryptInit_ex(&ctx, EVP_aes_128_ecb(), nullptr, key, nullptr) != 0;
	case 192:
		return EVP_EncryptInit_ex(&ctx, EVP_aes_192_ecb(), nullptr, key, nullptr) != 0;
	case 256:
		return EVP_EncryptInit_ex(&ctx, EVP_aes_256_ecb(), nullptr, key, nullptr) != 0;
	default:
		return false;
	}
}

int64_t Cpu::run(Bench::Container &sample)
{
#ifdef _DEBUG
#endif

	assert(sample.data != nullptr);

	unsigned char *inBuffer = sample.data;
	int inSize = static_cast<int>(sample.length);
	unsigned char *outBuffer = new unsigned char[sample.length * sizeof(*sample.data)];
	int outSize = static_cast<int>(sample.length);

	assert(inBuffer != nullptr);
	assert(outBuffer != nullptr);

	auto start = hrc::now();

	int status = EVP_EncryptUpdate(&ctx, outBuffer, &outSize, inBuffer, inSize);

	auto end = hrc::now();

	delete[] outBuffer;

	if (status)
	{
		return ch::duration_cast<ch::microseconds>(end - start).count();
	}
	
	return -1;
}

bool Cpu::release()
{
	EVP_CIPHER_CTX_cleanup(&ctx);

	return true;
}

Cpu::~Cpu()
{
}

}

std::unique_ptr<Aes::Cpu> factory()
{
	return std::unique_ptr<Aes::Cpu>(new Aes::Cpu());
}

}