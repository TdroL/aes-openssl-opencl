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

#ifndef KEYLENGTH
	#define KEYLENGTH(keybits) ((keybits)/8)
#endif
#ifndef RKLENGTH
	#define RKLENGTH(keybits)  ((keybits)/8+28)
#endif
#ifndef NROUNDS
	#define NROUNDS(keybits)   ((keybits)/32+6)
#endif

namespace Bench
{
namespace Aes
{

using namespace std;
namespace ch = boost::chrono;
namespace po = boost::program_options;
typedef ch::high_resolution_clock hrc;

struct KernelData
{
	string name;
	size_t keyLength;
};

const array<KernelData, 15> kernels = {{
	{ "aes_encrypt_rijndael_128", 128 },
	{ "aes_encrypt_rijndael_192", 192 },
	{ "aes_encrypt_rijndael_256", 256 },
	{ "aes_encrypt_rijndael_256_constant_roundkeys", 256 },
	{ "aes_encrypt_rijndael_256_local", 256 },
	{ "aes_encrypt_rijndael_256_local_roundkeys", 256 },
	{ "aes_encrypt_rijndael_256_opt", 256 },
	{ "aes_encrypt_rijndael_256_opt_constant_roundkeys", 256 },
	{ "aes_encrypt_rijndael_256_opt_local_roundkeys", 256 },
	{ "aes_encrypt_vector_128", 128 },
	{ "aes_encrypt_vector_128_local", 128 },
	{ "aes_encrypt_vector_192", 192 },
	{ "aes_encrypt_vector_192_local", 192 },
	{ "aes_encrypt_vector_256", 256 },
	{ "aes_encrypt_vector_256_local", 256 },
}};

Gpu::Gpu()
	: err(0),
	  platformId(nullptr),
	  deviceId(nullptr),
	  context(nullptr),
	  queue(nullptr),
	  program(nullptr),
	  kernel(nullptr),
	  memState(nullptr),
	  memRoundKeys(nullptr),
	  roundKeys(nullptr)
{
	string kernelsList;

	if ( ! kernels.empty())
	{
		for (size_t i = 0, l =  kernels.size() - 1; i < l; i++)
		{
			kernelsList += kernels[i].name;
			kernelsList += ", ";
		}

		kernelsList += kernels[kernels.size() - 1].name;
	}
	else
	{
		kernelsList = "none";
	}

	assert(desc != nullptr);
	desc->add_options()
		("kernel-name,k", po::value<string>()->default_value("aes_encrypt_rijndael_256_opt_local_roundkeys"),
						  ("kernel name (aes-gpu only), avaible: " + kernelsList).c_str())
		;
}

bool Gpu::init(size_t sampleLength)
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
	array<char, 1024 * 10> errorBuffer; // error message buffer, 10 KiB

	clGetProgramBuildInfo(program, deviceId, CL_PROGRAM_BUILD_LOG, errorBuffer.size(), errorBuffer.data(), &len);

	// trim buffer
	errMsg = errorBuffer.data();
	boost::algorithm::trim(errMsg);

	if ( ! errMsg.empty())
	{
		return false;
	}

	string kernelName = (*vm)["kernel-name"].as<string>();
	auto kernelId = find_if(kernels.begin(), kernels.end(), [&kernelName] (const KernelData &data) -> bool {
		return  (data.name == kernelName);
	});

	if (kernelId == kernels.end())
	{
		errMsg = "Unknown kernel name";
		return false;
	}

	keyLength = kernelId->keyLength;
	cout << "Kernel name: " << kernelName << endl;
	cout << "Key length: " << keyLength << " bits" << endl;

	kernel = clCreateKernel(program, kernelId->name.c_str(), &err);
	if ( ! kernel || err != CL_SUCCESS) {
		errMsg = (boost::format("Failed to create compute kernel (%1)") % kernelId->name).str();
		return false;
	}

	if (sampleLength < 16) // 128 bit / 8 = 16 bytes
	{
		errMsg = "State length too small, must be at least 16 bytes (128 bits) long";
		return false;
	}

	/* *
	 * Number of work-items in work-group: 256
	 * State size: 128 bits (16 bytes)
	 * Smallest posible sample: 256 * 16 bytes = 4096 bytes
	 *
	 * sampleLengthPadded = ((sampleLength / 16 + 256 - 1) / 256) * 256 * 16;
	 */
	sampleLengthPadded = sampleLength;

	if (sampleLengthPadded % 4096 != 0)
	{
		sampleLengthPadded += 4096 - sampleLengthPadded % 4096;
	}

	cout << "Sample size padded: " << sampleLengthPadded << " bytes" << endl;

	memState = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR, 
							   sampleLengthPadded, NULL, &err);
	if (err != CL_SUCCESS)
	{
		errMsg = (boost::format("Failed to allocate device memory (state, size: %1)") % sampleLength).str();
		return false;
	}

	memRoundKeys = clCreateBuffer(context, CL_MEM_READ_ONLY, RKLENGTH(keyLength), NULL, &err);
	if (err != CL_SUCCESS)
	{
		errMsg = (boost::format("Failed to allocate device memory (roundkeys, size: %1)") % RKLENGTH(keyLength)).str();
		return false;
	}

	err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &memState);
	if (err != CL_SUCCESS)
	{
		errMsg = "Failed to bind kernel argument \"mem_state\" [#0]";
		return false;
	}

	err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &memRoundKeys);
	if (err != CL_SUCCESS)
	{
		errMsg = "Failed to bind kernel argument \"mem_state\" [#1]";
		return false;
	}

	char key[KEYLENGTH(256)] = "tajny klucz";
	roundKeys.reset(new uint32_t[RKLENGTH(keyLength)]);

	rijndaelSetupEncrypt(reinterpret_cast<unsigned long *>(roundKeys.get()), reinterpret_cast<const uint8_t *>(key), keyLength);

	return true;
}

int64_t Gpu::run(Bench::Container &sample)
{
	unique_ptr<unsigned char> outBuffer(new unsigned char[sample.length * sizeof(sample.data[0])]);

	auto start = hrc::now();

	size_t local = 256;
	size_t global = sampleLengthPadded / 16;

	err = clEnqueueWriteBuffer(queue, memRoundKeys, CL_FALSE, 0, RKLENGTH(keyLength), 
							   roundKeys.get(), 0, NULL, NULL);
	if (err != CL_SUCCESS) {
		errMsg = "Failed write data to buffer (roundkeys)";
		return -1;
	}

	err = clEnqueueWriteBuffer(queue, memState, CL_FALSE, 0, sample.length, sample.data, 
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

	err = clEnqueueReadBuffer(queue, memState, CL_FALSE, 0, sample.length, sample.data,
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