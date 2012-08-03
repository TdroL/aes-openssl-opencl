#ifndef BENCH_AES_GPU_H
#define BENCH_AES_GPU_H

#include "../base.h"

namespace Bench
{
namespace Aes
{

class Gpu : public Bench::Base
{
public:
	bool init();
	int64_t run(Bench::Base::data_type data, size_t size);
	bool release();

	~Gpu();
};

}
}

#endif