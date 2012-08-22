#include "stdafx.h"
#include "gpu.h"
#include "boost/algorithm/string.hpp"
#include "boost/format.hpp"
#include "boost/chrono.hpp"
#include "boost/thread.hpp"
#include <iostream>
#include <sstream>
#include <fstream>
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
	  memState(nullptr),
	  memRoundKeys(nullptr),
	  roundKeys(nullptr)
{
}

void Gpu::add_options()
{
	desc->add_options()
		("kernel-name,n", po::value<string>()->default_value("aes"),
							"kernel name (aes-gpu only)")
		("work-items,i",  po::value<size_t>()->default_value(256),
							"work-items per work-group (aes-gpu only)")
		;
}

bool Gpu::opencl_init()
{
	err = clGetPlatformIDs(1, &platformId, NULL);
	if (err != CL_SUCCESS)
	{
		errMsg = (boost::format("Could not connect to compute device (err %1)") % perror(err)).str();
		return false;
	}

	err = clGetDeviceIDs(platformId, CL_DEVICE_TYPE_GPU, 1, &deviceId, NULL);
	if (err != CL_SUCCESS)
	{
		errMsg = (boost::format("Could not get device id (err %1)") % perror(err)).str();
		return false;
	}

	context = clCreateContext(0, 1, &deviceId, NULL, NULL, &err);
	if ( ! context || err != CL_SUCCESS)
	{
		errMsg = (boost::format("Failed to create a compute context (err %1)") % perror(err)).str();
		return false;
	}

	queue = clCreateCommandQueue(context, deviceId, 0, &err);
	if ( ! queue || err != CL_SUCCESS)
	{
		errMsg = (boost::format("Failed to create a command queue (err %1)") % perror(err)).str();
		return false;
	}

	return true;
}

bool Gpu::opencl_load_source(string &file)
{
	stringstream sourceBuffer;
	sourceBuffer << "__constant uint timestamp = " << time(nullptr) << ";" << endl; // prevent caching
	// load required headers
	{
		fstream fs(includePath + "aes_encrypt_tables.cl");
		sourceBuffer << fs.rdbuf() << endl;
	}
	// load requested kernel
	{
		fstream fs(includePath + file + ".cl");

		if ( ! fs.is_open() || ! fs.good())
		{
			errMsg = "Failed to load requested kernel";
			return false;
		}

		sourceBuffer << fs.rdbuf() << endl;
	}

	const string &sourceString = sourceBuffer.str();
	const char *sourceCStr = sourceString.c_str();

	program = clCreateProgramWithSource(context, 1, &sourceCStr, NULL, &err);
	if ( ! program || err != CL_SUCCESS)
	{
		errMsg = (boost::format("Failed to create compute program (err %1)") % perror(err)).str();
		return false;
	}

	return true;
}

bool Gpu::opencl_build(string &kernelName, string &options)
{
	cout << "Build options: " << options << endl;

	err = clBuildProgram(program, 0, NULL, options.c_str(), NULL, NULL);
	size_t len;
	array<char, 1024 * 10> logBuffer; // error message buffer, 10 KiB

	err = clGetProgramBuildInfo(program, deviceId, CL_PROGRAM_BUILD_LOG, logBuffer.size(), logBuffer.data(), &len);

	// trim buffer
	errMsg = logBuffer.data();
	boost::algorithm::trim(errMsg);

	if (err != CL_SUCCESS)
	{
		return false;
	}

	if (errMsg.size() > 0)
	{
		cout << "Build log:" << endl << errMsg << endl;
	}

	kernel = clCreateKernel(program, kernelName.c_str(), &err);
	if ( ! kernel || err != CL_SUCCESS) {
		errMsg = (boost::format("Failed to create compute kernel %1 (err %2)") % kernelName % perror(err)).str();
		return false;
	}

	return true;
}

bool Gpu::init(size_t sampleLength, size_t keyLength_)
{
	if ( ! opencl_init())
	{
		return false;
	}

	string kernelName = (*vm)["kernel-name"].as<string>();

	keyLength = keyLength_;
	cout << "Kernel name: " << kernelName << endl;
	cout << "Key length: " << keyLength << " bits" << endl;

	if ( ! opencl_load_source(kernelName))
	{
		return false;
	}

	stringstream options;
	options << "-D NROUNDS=" << NROUNDS(keyLength_) << " -cl-nv-verbose";

#ifdef OPENCL_UNSAFE_OPTIMIZATIONS
	options << " -cl-mad-enable -cl-unsafe-math-optimizations";
#endif

	if ( ! opencl_build(kernelName, options.str()))
	{
		return false;
	}

	if (sampleLength < 16) // 128 bit / 8 = 16 bytes
	{
		errMsg = "State length too small, must be at least 16 bytes (128 bits) long";
		return false;
	}

	local = (*vm)["work-items"].as<size_t>();

	if (local == 0)
	{
		cout << "  Forcing at least 1 work-item per work-group" << endl;
	}

	if (kernelName.find("local") != string::npos && local != 256)
	{
		cout << "  Forcing 256 work-items per work-group" << endl;
	}

	/* *
	 * Case #1: no local data in kernel
	 * Case #2: local data in kernel
	 * Number of work-items in work-group: 256
	 * State size: 128 bits (16 bytes)
	 * Smallest posible sample length: 256 * 16 bytes = 4096 bytes
	 *
	 * sampleLengthPadded = ((sampleLength / 16 + 256 - 1) / 256) * 256 * 16;
	 */
	sampleLengthPadded = sampleLength;

	size_t lengthPad = 16 * local;

	if (sampleLengthPadded % lengthPad != 0)
	{
		sampleLengthPadded += lengthPad - (sampleLengthPadded % lengthPad);
	}

	global = sampleLengthPadded / 16;

	cout << "Sample size padded: " << sampleLengthPadded << " bytes (padding to " << lengthPad << " bytes)" << endl;

	memState = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR,
							   sampleLengthPadded, NULL, &err);
	if (err != CL_SUCCESS)
	{
		errMsg = (boost::format("Failed to allocate device memory (state, size: %1; %2)") % sampleLength % perror(err)).str();
		return false;
	}

	memRoundKeys = clCreateBuffer(context, CL_MEM_READ_ONLY, RKLENGTH(256), NULL, &err);
	if (err != CL_SUCCESS)
	{
		errMsg = (boost::format("Failed to allocate device memory (roundkeys, size: %1; %2)") % RKLENGTH(256) % perror(err)).str();
		return false;
	}

	err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &memState);
	if (err != CL_SUCCESS)
	{
		errMsg = (boost::format("Failed to bind kernel argument \"memState\" (%1)") % perror(err)).str();
		return false;
	}

	err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &memRoundKeys);
	if (err != CL_SUCCESS)
	{
		errMsg = (boost::format("Failed to bind kernel argument \"memRoundKeys\" (%1)") % perror(err)).str();
		return false;
	}

	unsigned char key[] = { 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16 };
	roundKeys.reset(new uint32_t[RKLENGTH(keyLength)]);

	rijndaelSetupEncrypt(reinterpret_cast<unsigned long *>(roundKeys.get()), key, keyLength);

	err = clEnqueueWriteBuffer(queue, memRoundKeys, CL_TRUE, 0, RKLENGTH(keyLength),
							   roundKeys.get(), 0, NULL, NULL);
	if (err != CL_SUCCESS) {
		errMsg = (boost::format("Failed write data to buffer \"roundkeys\" (%1)") % perror(err)).str();
		return false;
	}

	return true;
}

int64_t Gpu::run(Bench::Container &sample)
{
	unique_ptr<unsigned char> outBuffer(new unsigned char[sample.length * sizeof(sample.data[0])]);

	unsigned int nrounds = NROUNDS(keyLength);
	assert(nrounds == 10 || nrounds == 12 || nrounds == 14);

	auto start = hrc::now();

	err = clEnqueueWriteBuffer(queue, memState, CL_FALSE, 0, sample.length, sample.data,
							   0, NULL, NULL);
	if (err != CL_SUCCESS) {
		errMsg = "Failed write data to buffer (state)";
		return -1;
	}

	err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global, &local, 0, NULL, NULL);
	if (err != CL_SUCCESS) {
		errMsg = (boost::format("Failed to execute kernel (%1)") % perror(err)).str();
		return -1;
	}

	err = clEnqueueReadBuffer(queue, memState, CL_FALSE, 0, sample.length, sample.data,
							  0, NULL, NULL);
	if (err != CL_SUCCESS) {
		errMsg = (boost::format("Failed to read from buffer (%1)") % perror(err)).str();
		return -1;
	}

	err = clFinish(queue);
	if (err != CL_SUCCESS) {
		errMsg = (boost::format("Failed to finish queue (%1)") % perror(err)).str();
		return -1;
	}

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

string Gpu::perror(cl_int err)
{
	static array<string, 47> errors = {{
		"CL_SUCCESS",
		"CL_DEVICE_NOT_FOUND",
		"CL_DEVICE_NOT_AVAILABLE",
		"CL_COMPILER_NOT_AVAILABLE",
		"CL_MEM_OBJECT_ALLOCATION_FAILURE",
		"CL_OUT_OF_RESOURCES",
		"CL_OUT_OF_HOST_MEMORY",
		"CL_PROFILING_INFO_NOT_AVAILABLE",
		"CL_MEM_COPY_OVERLAP",
		"CL_IMAGE_FORMAT_MISMATCH",
		"CL_IMAGE_FORMAT_NOT_SUPPORTED",
		"CL_BUILD_PROGRAM_FAILURE",
		"CL_MAP_FAILURE",
		"CL_INVALID_VALUE",
		"CL_INVALID_DEVICE_TYPE",
		"CL_INVALID_PLATFORM",
		"CL_INVALID_DEVICE",
		"CL_INVALID_CONTEXT",
		"CL_INVALID_QUEUE_PROPERTIES",
		"CL_INVALID_COMMAND_QUEUE",
		"CL_INVALID_HOST_PTR",
		"CL_INVALID_MEM_OBJECT",
		"CL_INVALID_IMAGE_FORMAT_DESCRIPTOR",
		"CL_INVALID_IMAGE_SIZE",
		"CL_INVALID_SAMPLER",
		"CL_INVALID_BINARY",
		"CL_INVALID_BUILD_OPTIONS",
		"CL_INVALID_PROGRAM",
		"CL_INVALID_PROGRAM_EXECUTABLE",
		"CL_INVALID_KERNEL_NAME",
		"CL_INVALID_KERNEL_DEFINITION",
		"CL_INVALID_KERNEL",
		"CL_INVALID_ARG_INDEX",
		"CL_INVALID_ARG_VALUE",
		"CL_INVALID_ARG_SIZE",
		"CL_INVALID_KERNEL_ARGS",
		"CL_INVALID_WORK_DIMENSION",
		"CL_INVALID_WORK_GROUP_SIZE",
		"CL_INVALID_WORK_ITEM_SIZE",
		"CL_INVALID_GLOBAL_OFFSET",
		"CL_INVALID_EVENT_WAIT_LIST",
		"CL_INVALID_EVENT",
		"CL_INVALID_OPERATION",
		"CL_INVALID_GL_OBJECT",
		"CL_INVALID_BUFFER_SIZE",
		"CL_INVALID_MIP_LEVEL",
		"CL_INVALID_GLOBAL_WORK_SIZE"
	}};

	size_t id = -err;

	if (id < errors.size())
	{
		return errors[id];
	}

	return (boost::format("Unknown error (%1)") % err).str();
}

Gpu::~Gpu()
{
}

}

std::unique_ptr<Aes::Gpu> factory()
{
	static bool added = false;

	if ( ! added)
	{
		assert(Base::desc != nullptr);
		Aes::Gpu::add_options();
		added = true;
	}

	return std::unique_ptr<Aes::Gpu>(new Aes::Gpu());
}

}