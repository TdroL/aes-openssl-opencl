#ifndef BENCH_AES_CPU_H
#define BENCH_AES_CPU_H

#include "../base.h"
#include <openssl/evp.h>

namespace Bench
{
namespace Aes
{

class Cpu : public Bench::Base
{
public:
	EVP_CIPHER_CTX ctx;

	static void add_options();

	bool init(size_t sampleLength, size_t keyLength);
	std::string run(Bench::Container &sample);
	bool release();

	~Cpu();
};

}

}

#endif