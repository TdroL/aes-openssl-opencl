#ifndef BENCH_AES_GPU_H
#define BENCH_AES_GPU_H

#include "../base.h"
#include <CL/cl.h>
#include "../../rijndael.h"

namespace Bench
{
namespace Aes
{

static const std::string includePath = "./kernels/";

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
	cl_mem memState;
	cl_mem memRoundKeys;
	size_t sampleLengthPadded;
	size_t keyLength;
	uint32_t roundKeys[RKLENGTH(256)];
	size_t local, global;

	Gpu();

	static void add_options();

	virtual bool opencl_init();
	virtual bool opencl_load_source(std::string &file);
	virtual bool opencl_load_ptx(std::string &file);
	virtual bool opencl_save_ptx(std::string &file);
	virtual bool opencl_build(std::string &kernelName, std::string &options = std::string(""));

	virtual bool init(size_t sampleLength, size_t keyLength);
	virtual std::string run(Bench::Container &sample);
	virtual bool release();

	template <typename T>
	void get_device_info(cl_device_info deviceInfo, T &value);

	template <typename T>
	void get_device_info(cl_device_info deviceInfo, T* values, size_t count);

	template <>
	void get_device_info(cl_device_info deviceInfo, std::string &value);

	std::string pparam_name(cl_device_info paramName, bool trim = true);
	std::string perror(cl_int err, bool trim = true);

	~Gpu();
};

}
}

#endif