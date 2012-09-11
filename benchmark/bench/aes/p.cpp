#include "stdafx.h"
#include "p.h"
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

void P::add_options()
{
}

bool P::init(size_t sampleLength, size_t keyLength_)
{
	EVP_CIPHER_CTX_init(&ctx);

	keyLength = keyLength_;

	return (keyLength == 128 || keyLength == 192 || keyLength == 256);
}

string P::run(Bench::Container &sample)
{
	assert(sample.data != nullptr);

	typedef Bench::Container::data_type data_type;

	data_type *inBuffer = sample.data;
	int inSize = static_cast<int>(sample.length);

	unique_ptr<data_type[]> outBuffer(new data_type[sample.length]);
	int outSize = static_cast<int>(sample.length);

	assert(inBuffer != nullptr);
	assert(outBuffer != nullptr);

	unsigned char key[] = { 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16 };
	EVP_CIPHER *cipher = nullptr;

	switch (keyLength)
	{
	case 128:
		cipher = const_cast<EVP_CIPHER *>(EVP_aes_128_ecb());
		break;
	case 192:
		cipher = const_cast<EVP_CIPHER *>(EVP_aes_192_ecb());
		break;
	case 256:
		cipher = const_cast<EVP_CIPHER *>(EVP_aes_256_ecb());
		break;
	}

	if ( ! cipher)
	{
		return "";
	}

	int status = 1;

	auto start_init = hrc::now();

	for (size_t i = 0; i < 100; i++)
	{
		status &= EVP_EncryptInit_ex(&ctx, cipher, nullptr, key, nullptr);
	}

	auto end_init = hrc::now();

	if ( ! status)
	{
		return "";
	}

	auto start_update = hrc::now();

	for (size_t i = 0; i < 100; i++)
	{
		status &= EVP_EncryptUpdate(&ctx, outBuffer.get(), &outSize, inBuffer, inSize);
	}

	auto end_update = hrc::now();

	if (status)
	{
		return boost::lexical_cast<string>(ch::duration_cast<ch::microseconds>(end_init - start_init).count())
		       + "	" +
		       boost::lexical_cast<string>(ch::duration_cast<ch::microseconds>(end_update - start_update).count());
	}

	return "";
}

bool P::release()
{
	EVP_CIPHER_CTX_cleanup(&ctx);

	return true;
}

P::~P()
{
}

}

std::unique_ptr<Aes::P> factory()
{
	static bool added = false;

	if ( ! added)
	{
		assert(Base::desc != nullptr);
		Aes::P::add_options();
		added = true;
	}

	return std::unique_ptr<Aes::P>(new Aes::P());
}

}