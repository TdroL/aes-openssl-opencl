#ifndef BENCH_AES_P_GPU_H
#define BENCH_AES_P_GPU_H

#include "../gpu.h"

namespace Bench
{
namespace Aes
{
namespace P
{

class Gpu : public Bench::Aes::Gpu
{
public:
	Gpu();

	virtual bool init(size_t sampleLength, size_t keyLength);
	virtual std::string run(Bench::Container &sample);
	virtual bool release();

	~Gpu();
};

}

}

}

#endif