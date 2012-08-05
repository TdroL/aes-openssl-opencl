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

	bool init();
	int64_t run(Bench::Container &sample);
	bool release();

	~Cpu();
};

}

}

#endif