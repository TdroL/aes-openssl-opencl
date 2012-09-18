#include "stdafx.h"
#include "gpu.h"
#include "boost/algorithm/string.hpp"
#include "boost/format.hpp"
#include "boost/chrono.hpp"
#include "boost/thread.hpp"
#include "boost/type_traits.hpp"
#include "boost/lexical_cast.hpp"
#include "boost/filesystem.hpp"
#include <iostream>
#include <sstream>
#include <fstream>

namespace Bench
{
namespace Aes
{
namespace P
{

using namespace std;
namespace ch = boost::chrono;
namespace po = boost::program_options;
typedef ch::high_resolution_clock hrc;

Gpu::Gpu()
{
}

bool Gpu::init(size_t sampleLength, size_t keyLength_)
{
	keyLength = keyLength_;
	sampleLengthPadded = sampleLength;
	return true;
}

std::string Gpu::run(Bench::Container &sample)
{
	string kernelName = "aes_vector_unroll_local_roundkeys_local_tables_if";

	stringstream ssoptions;
	ssoptions << "-D NROUNDS=" << NROUNDS(keyLength);
	string options = ssoptions.str();

	auto start = hrc::now();

	if ( ! opencl_init())
	{
		return "";
	}

	if (vm->count("load-ptx"))
	{
		string file = (*vm)["load-ptx"].as<string>();

		opencl_load_ptx(file);
	}
	else
	{
		opencl_load_source(kernelName);
	}

	if ( ! program)
	{
		errMsg = "Could not load program source";
		return "";
	}

	if ( ! opencl_build(kernelName, options))
	{
		errMsg = "Could not build program";
		return "";
	}

	cl_ulong maxAllocSize;
	err = clGetDeviceInfo(deviceId, CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(maxAllocSize), &maxAllocSize, nullptr);
	if (err != CL_SUCCESS)
	{
		errMsg = (boost::format("Failed to get maximum allocable memory size (err %1%)") % perror(err)).str();
		return "";
	}

	memState = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR, sampleLengthPadded, nullptr, &err);
	if (err != CL_SUCCESS || memState == nullptr)
	{
		errMsg = (boost::format("Failed to allocate device memory (state, size: %1%; %2%)") % sampleLengthPadded % perror(err)).str();
		return "";
	}

	memRoundKeys = clCreateBuffer(context, CL_MEM_READ_ONLY, RKLENGTH(256), nullptr, &err);
	if (err != CL_SUCCESS || memRoundKeys == nullptr)
	{
		errMsg = (boost::format("Failed to allocate device memory (roundkeys, size: %1%; %2%)") % RKLENGTH(256) % perror(err)).str();
		return "";
	}

	err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &memState);
	if (err != CL_SUCCESS)
	{
		errMsg = (boost::format("Failed to bind kernel argument \"memState\" (%1%)") % perror(err)).str();
		return "";
	}

	err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &memRoundKeys);
	if (err != CL_SUCCESS)
	{
		errMsg = (boost::format("Failed to bind kernel argument \"memRoundKeys\" (%1%)") % perror(err)).str();
		return "";
	}

	unsigned char key[KEYLENGTH(256)] = { 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32 };

	rijndaelSetupEncrypt(reinterpret_cast<unsigned long *>(roundKeys), key, keyLength);

	err = clEnqueueWriteBuffer(queue, memRoundKeys, CL_TRUE, 0, RKLENGTH(keyLength), roundKeys, 0, nullptr, nullptr);
	if (err != CL_SUCCESS)
	{
		errMsg = (boost::format("Failed write data to buffer \"roundkeys\" (%1%)") % perror(err)).str();
		return "";
	}

	auto end = hrc::now();

	if (memState != nullptr)
	{
		while (clReleaseMemObject(memState));
		memState = nullptr;
	}

	if (memRoundKeys != nullptr)
	{
		while (clReleaseMemObject(memRoundKeys));
		memRoundKeys = nullptr;
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

	return boost::lexical_cast<string>(ch::duration_cast<ch::microseconds>(end - start).count());
}

bool Gpu::release()
{
	return true;
}

Gpu::~Gpu()
{
}

}

}

std::unique_ptr<Aes::P::Gpu> factory()
{
	return std::unique_ptr<Aes::P::Gpu>(new Aes::P::Gpu);
}

}