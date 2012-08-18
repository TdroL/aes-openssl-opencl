#include "stdafx.h"
#include "gpuloop.h"
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

namespace Perf
{

using namespace std;
namespace ch = boost::chrono;
namespace po = boost::program_options;
typedef ch::high_resolution_clock hrc;

const array<const string, 1> kernels = {{
	"aes_loop_test"
}};

GpuLoop::GpuLoop()
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

	assert(desc != nullptr);
	desc->add_options()
		("loop-type,t", po::value<string>()->default_value("macro"),
						  "loop type (perf-gpuloop only), avaible:\nmacro, variable")
		;
}

bool GpuLoop::init(size_t sampleLength, size_t keyLength_)
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

	stringstream sourceBuffer;
	sourceBuffer << "__constant uint timestamp = " << time(nullptr) << ";" << endl; // prevent caching\
	// for required headers
	{
		fstream fs(string(includePath) + "aes_encrypt_tables.cl");
		sourceBuffer << fs.rdbuf() << endl;
	}
	for (auto it = kernels.begin(); it != kernels.end(); ++it)
	{
		//sourceBuffer << "#include \"" << includePath << *it << ".cl\"" << endl;

		fstream fs(includePath + *it + ".cl");
		sourceBuffer << fs.rdbuf() << endl;
	}

	string sourceString = sourceBuffer.str();
	const char *sourceCStr = sourceString.c_str();

	program = clCreateProgramWithSource(context, 1, &sourceCStr, NULL, &err);
	if ( ! program || err != CL_SUCCESS)
	{
		errMsg = "Failed to create compute program";
		return false;
	}

	stringstream options;
	string loopType = (*vm)["loop-type"].as<string>();

	if (loopType == "macro")
	{
		options << " -D NROUNDS=" << NROUNDS(keyLength_);
	}

	options << " -cl-nv-verbose";

#ifdef OPENCL_UNSAFE_OPTIMIZATIONS
	options << " -cl-mad-enable -cl-unsafe-math-optimizations";
#endif

	cout << "Build options: " << options.str() << endl;

	err = clBuildProgram(program, 0, NULL, options.str().c_str(), NULL, NULL);
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

	string kernelName = "aes_loop_test";
	auto kernelId = find(kernels.begin(), kernels.end(), kernelName);

	if (kernelId == kernels.end())
	{
		errMsg = "Unknown kernel name";

		string kernelsList;
		if ( ! kernels.empty())
		{
			for (size_t i = 0, l =  kernels.size() - 1; i < l; i++)
			{
				kernelsList += kernels[i];
				kernelsList += ", ";
			}

			kernelsList += kernels[kernels.size() - 1];
		}
		else
		{
			kernelsList = "none";
		}

		errMsg += "\nAvaible kernels: " + kernelsList;
		return false;
	}

	keyLength = keyLength_;
	cout << "Kernel name: " << kernelName << endl;
	cout << "Key length: " << keyLength << " bits" << endl;

	kernel = clCreateKernel(program, kernelName.c_str(), &err);
	if ( ! kernel || err != CL_SUCCESS) {
		errMsg = (boost::format("Failed to create compute kernel (%1)") % kernelName).str();
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
	 * Smallest posible sample length: 256 * 16 bytes = 4096 bytes
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
		errMsg = "Failed to bind kernel argument \"memState\"";
		return false;
	}

	err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &memRoundKeys);
	if (err != CL_SUCCESS)
	{
		errMsg = "Failed to bind kernel argument \"memRoundKeys\"";
		return false;
	}

	cl_uint nrounds = NROUNDS(keyLength_);
	err = clSetKernelArg(kernel, 2, sizeof(nrounds), &nrounds);
	if (err != CL_SUCCESS)
	{
		errMsg = "Failed to bind kernel argument \"nrounds\"";
		return false;
	}	

	unsigned char key[] = { 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16 };
	roundKeys.reset(new uint32_t[RKLENGTH(keyLength)]);

	rijndaelSetupEncrypt(reinterpret_cast<unsigned long *>(roundKeys.get()), key, keyLength);

	return true;
}

int64_t GpuLoop::run(Bench::Container &sample)
{
	unique_ptr<unsigned char> outBuffer(new unsigned char[sample.length * sizeof(sample.data[0])]);

	auto start = hrc::now();

	size_t local = 256;
	size_t global = sampleLengthPadded / 16;
	unsigned int nrounds = NROUNDS(keyLength);

	assert(nrounds == 10 || nrounds == 12 || nrounds == 14);

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

	err = clFinish(queue);
	if (err != CL_SUCCESS) {
		errMsg = "Failed to finish queue";
		return -1;
	}

	auto end = hrc::now();
	return ch::duration_cast<ch::microseconds>(end - start).count();
}

bool GpuLoop::release()
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

GpuLoop::~GpuLoop()
{
}

}

std::unique_ptr<Perf::GpuLoop> factory()
{
	return std::unique_ptr<Perf::GpuLoop>(new Perf::GpuLoop());
}

}