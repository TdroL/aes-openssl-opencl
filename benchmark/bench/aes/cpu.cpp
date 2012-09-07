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

void Cpu::add_options()
{
}

bool Cpu::init(size_t sampleLength, size_t keyLength)
{
	unsigned char key[KEYLENGTH(256)] = { 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32 };

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

string Cpu::run(Bench::Container &sample)
{
#ifdef _DEBUG
#endif

	assert(sample.data != nullptr);

	unsigned char *inBuffer = sample.data;
	int inSize = static_cast<int>(sample.length);
	int outSize = static_cast<int>(sample.length);

	assert(inBuffer != nullptr);

	auto start = hrc::now();

	int status = EVP_EncryptUpdate(&ctx, inBuffer, &outSize, inBuffer, inSize);

	auto end = hrc::now();

	if (status)
	{
		return boost::lexical_cast<string>(ch::duration_cast<ch::microseconds>(end - start).count());
	}

	return "";
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
	static bool added = false;

	if ( ! added)
	{
		assert(Base::desc != nullptr);
		Aes::Cpu::add_options();
		added = true;
	}

	return std::unique_ptr<Aes::Cpu>(new Aes::Cpu());
}

}