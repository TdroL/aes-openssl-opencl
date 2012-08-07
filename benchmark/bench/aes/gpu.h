#ifndef BENCH_AES_GPU_H
#define BENCH_AES_GPU_H

#include "../base.h"
#include <CL/cl.h>

namespace Bench
{
namespace Aes
{

static const char pathToSource[] = "kernels/aes_encrypt.cl";

class Gpu : public Bench::Base
{
public:
	cl_int err;
	cl_platform_id platformId;
	cl_device_id deviceId;
	cl_context context;
	cl_command_queue queue;
	cl_program program;
	cl_kernel kernel;
	cl_mem mem_state;
	cl_mem mem_roundkeys;
	size_t sampleLengthPadded;
	unsigned char *roundkeys[240];

	Gpu();

	bool init(size_t sample_length);
	int64_t run(Bench::Container &sample);
	bool release();

	~Gpu();
};

}
}

#endif