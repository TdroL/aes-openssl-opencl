#ifndef BENCH_PERF_GPULOOP_H
#define BENCH_PERF_GPULOOP_H

#include "../base.h"
#include <CL/cl.h>

namespace Bench
{

namespace Perf
{

static const char includePath[] = "./kernels/";

class GpuLoop : public Bench::Base
{
public:
	cl_int err;
	cl_platform_id platformId;
	cl_device_id deviceId;
	cl_context context;
	cl_command_queue queue;
	cl_program program;
	cl_kernel kernel;
	cl_mem memState;
	cl_mem memRoundKeys;
	size_t sampleLengthPadded;
	size_t keyLength;
	std::unique_ptr<uint32_t> roundKeys;

	GpuLoop();

	bool init(size_t sampleLength, size_t keyLength);
	int64_t run(Bench::Container &sample);
	bool release();

	~GpuLoop();
};

}

}

#endif