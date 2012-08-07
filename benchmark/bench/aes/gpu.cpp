#include "stdafx.h"
#include "gpu.h"
#include "boost/algorithm/string.hpp"
#include "boost/format.hpp"
#include "boost/chrono.hpp"
#include "boost/thread.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include "../../rijndael.h"

namespace Bench
{
namespace Aes
{

using namespace std;
namespace ch = boost::chrono;
namespace po = boost::program_options;
typedef ch::high_resolution_clock hrc;

Gpu::Gpu()
	: err(0),
	  platformId(nullptr),
	  deviceId(nullptr),
	  context(nullptr),
	  queue(nullptr),
	  program(nullptr),
	  kernel(nullptr),
	  mem_state(nullptr),
	  mem_roundkeys(nullptr)
{
	string kernels = "aes_encrypt_rijndael_256, " \
					 "aes_encrypt_rijndael_256_constant_roundkeys, " \
					 "aes_encrypt_rijndael_256_local_roundkeys, " \
					 "aes_encrypt_rijndael_256_opt, " \
					 "aes_encrypt_rijndael_256_opt_constant_roundkeys, " \
					 "aes_encrypt_rijndael_256_opt_local_roundkeys, " \
					 "aes_encrypt_rijndael_256_local, " \
					 "aes_encrypt_vector_256, " \
					 "aes_encrypt_vector_256_local";

	assert(desc != nullptr);
	desc->add_options()
		("kernel-name,k", po::value<string>()->default_value("aes_encrypt_rijndael_256_opt_local_roundkeys"),
						  ("kernel name (aes-gpu only), avaible: " + kernels).c_str())
		;
}

bool Gpu::init(size_t sample_length)
{
	err = clGetPlatformIDs(1, &platformId, NULL);
	if (err != CL_SUCCESS) {
		errMsg = "Could not connect to compute device";
		return false;
	}

	err = clGetDeviceIDs(platformId, CL_DEVICE_TYPE_GPU, 1, &deviceId, NULL);
	if (err != CL_SUCCESS) {
		errMsg = "Could not get device id";
		return false;
	}

	context = clCreateContext(0, 1, &deviceId, NULL, NULL, &err);
	if ( ! context || err != CL_SUCCESS) {
		errMsg = "Failed to create a compute context";
		return false;
	}

	queue = clCreateCommandQueue(context, deviceId, 0, &err);
	if ( ! queue || err != CL_SUCCESS) {
		errMsg = "Failed to create a command queue";
		return false;
	}

	fstream fs(pathToSource, ios::in);
	fs.clear();

	if ( ! fs.is_open())
	{
		errMsg = "Failed to read source";
		return false;
	}

	stringstream sourceBuffer;
	copy(istreambuf_iterator<char>(fs),
		 istreambuf_iterator<char>(),
		 ostreambuf_iterator<char>(sourceBuffer));

	sourceBuffer << "\n//timestamp: " << time(NULL) << endl;

	string sourceString = sourceBuffer.str();
	const char *sourceCStr = sourceString.c_str();

	program = clCreateProgramWithSource(context, 1, &sourceCStr, NULL, &err);
	if ( ! program || err != CL_SUCCESS)
	{
		errMsg = "Failed to create compute program";
		return false;
	}

	const char options[] = "-cl-mad-enable -cl-unsafe-math-optimizations";
	err = clBuildProgram(program, 0, NULL, options, NULL, NULL);
	size_t len;
	char errorBuffer[1024 * 10]; // error message buffer, 10 KiB

	clGetProgramBuildInfo(program, deviceId, CL_PROGRAM_BUILD_LOG, sizeof(errorBuffer), errorBuffer, &len);

	// trim buffer
	errMsg = errorBuffer;
	boost::algorithm::trim(errMsg);

	if ( ! errMsg.empty())
	{
		return false;
	}

	kernel = clCreateKernel(program, (*vm)["kernel-name"].as<string>().c_str(), &err);
	if ( ! kernel || err != CL_SUCCESS) {
		errMsg = "Failed to create compute kernel";
		return false;
	}

	if (sample_length < 16) // 128 bit / 8 = 16 bytes
	{
		errMsg = "State length too small, must be at least 16 bytes (128 bits) long";
		return false;
	}

	sampleLengthPadded = ((sample_length / 16 + 256 - 1) / 256) * 256 * 16;

	mem_state = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR, 
							   sampleLengthPadded, NULL, &err);
	if (err != CL_SUCCESS)
	{
		errMsg = (boost::format("Failed to allocate device memory (state, size: %1)") % sample_length).str();
		return false;
	}

	mem_roundkeys = clCreateBuffer(context, CL_MEM_READ_ONLY, 240, NULL, &err);
	if (err != CL_SUCCESS)
	{
		errMsg = "Failed to allocate device memory (roundkeys, size: 240)";
		return false;
	}

	err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &mem_state);
	if (err != CL_SUCCESS)
	{
		errMsg = "Failed to bind kernel argument \"mem_state\" [#0]";
		return false;
	}

	err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &mem_roundkeys);
	if (err != CL_SUCCESS)
	{
		errMsg = "Failed to bind kernel argument \"mem_state\" [#1]";
		return false;
	}

	const unsigned char *key = reinterpret_cast<const unsigned char *>("tajny klucz");

	rijndaelSetupEncrypt(reinterpret_cast<unsigned long *>(roundkeys), key, 256);

	return true;
}

int64_t Gpu::run(Bench::Container &sample)
{
	unique_ptr<unsigned char> outBuffer(new unsigned char[sample.length * sizeof(sample.data[0])]);

	auto start = hrc::now();

	size_t local = 256;
	size_t global = sampleLengthPadded / 16;

	err = clEnqueueWriteBuffer(queue, mem_roundkeys, CL_FALSE, 0, 240, 
							   roundkeys, 0, NULL, NULL);
	if (err != CL_SUCCESS) {
		errMsg = "Failed write data to buffer (roundkeys)";
		return -1;
	}

	err = clEnqueueWriteBuffer(queue, mem_state, CL_FALSE, 0, sample.length, sample.data, 
							   0, NULL, NULL);
	if (err != CL_SUCCESS) {
		errMsg = "Failed write data to buffer (state)";
		return -1;
	}

	err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global, &local, 0, NULL, NULL);
	if (err != CL_SUCCESS) {
		errMsg = "Failed to execute kernel";
		return -1;
	}

	err = clEnqueueReadBuffer(queue, mem_state, CL_FALSE, 0, sample.length, sample.data,
							  0, NULL, NULL);
	if (err != CL_SUCCESS) {
		errMsg = "Failed to read from buffer";
		return -1;
	}

	clFinish(queue);

	auto end = hrc::now();
	return ch::duration_cast<ch::microseconds>(end - start).count();
}

bool Gpu::release()
{
	if (mem_state != nullptr)
	{
		while (clReleaseMemObject(mem_state));
		mem_state = nullptr;
	}
	
	if (mem_roundkeys != nullptr)
	{
		while (clReleaseMemObject(mem_roundkeys));
		mem_roundkeys = nullptr;
	}

	if (kernel != nullptr)
	{
		while (clReleaseKernel(kernel));
		kernel = nullptr;
	}

	if (program != nullptr)
	{
		while (clReleaseProgram(program));
		program = nullptr;
	}

	if (queue != nullptr)
	{
		while (clReleaseCommandQueue(queue));
		queue = nullptr;
	}

	if (context != nullptr)
	{
		while (clReleaseContext(context));
		context = nullptr;
	}

	return true;
}

Gpu::~Gpu()
{
}

}

std::unique_ptr<Aes::Gpu> factory()
{
	return std::unique_ptr<Aes::Gpu>(new Aes::Gpu);
}

}