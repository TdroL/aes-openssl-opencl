#ifndef BENCH_AES_P_H
#define BENCH_AES_P_H

#include "../base.h"
#include <openssl/evp.h>

namespace Bench
{
namespace Aes
{

class P : public Bench::Base
{
public:
	EVP_CIPHER_CTX ctx;
	size_t keyLength;

	static void add_options();

	bool init(size_t sampleLength, size_t keyLength);
	std::string run(Bench::Container &sample);
	bool release();

	~P();
};

}

}

#endif