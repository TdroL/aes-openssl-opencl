#ifndef BENCH_AES_CPU
#define BENCH_AES_CPU

#include "../base.h"

namespace Bench
{
namespace Aes
{

class Cpu : public Bench::Base
{
public:
	bool init();
	int64_t run(Bench::Base::data_type data, size_t size);
	bool release();

	~Cpu();
};

}
}

#endif