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
	  memRoundKeys(nullptr)
{
}

void Gpu::add_options()
{
	desc->add_options()
		("kernel-name,n", po::value<string>()->default_value("aes"),
							"kernel name (aes-gpu only)")
		("work-items,i",  po::value<size_t>()->default_value(256),
							"work-items per work-group (aes-gpu only)")
		("verbose",      "enables build logs (aes-gpu only)")
		("cache",        "enables kernel caching (aes-gpu only)")
		("device-info",  "prints device info (aes-gpu only)")
		("load-ptx",     po::value<string>(),
						 "loads pre-compiled kernel (aes-gpu only)")
		("save-ptx",     po::value<string>(),
						 "saves compiled kernel (aes-gpu only)")
		("unsafe-optimizations", "enables unsafe opencl optimizations (aes-gpu only)")
		;
}

bool Gpu::opencl_init()
{
	err = clGetPlatformIDs(1, &platformId, nullptr);
	if (err != CL_SUCCESS)
	{
		errMsg = (boost::format("Could not connect to compute device (err %1%)") % perror(err)).str();
		return false;
	}

	err = clGetDeviceIDs(platformId, CL_DEVICE_TYPE_GPU, 1, &deviceId, nullptr);
	if (err != CL_SUCCESS)
	{
		errMsg = (boost::format("Could not get device id (err %1%)") % perror(err)).str();
		return false;
	}

	context = clCreateContext(0, 1, &deviceId, nullptr, nullptr, &err);
	if ( ! context || err != CL_SUCCESS)
	{
		errMsg = (boost::format("Failed to create a compute context (err %1%)") % perror(err)).str();
		return false;
	}

	queue = clCreateCommandQueue(context, deviceId, 0, &err);
	if ( ! queue || err != CL_SUCCESS)
	{
		errMsg = (boost::format("Failed to create a command queue (err %1%)") % perror(err)).str();
		return false;
	}

	if (vm->count("device-info"))
	{
		cout << "Device info:" << endl;

		{
			string info;
			get_device_info(CL_DEVICE_NAME, info);
			cout << "  CL_DEVICE_NAME                     : " << info << endl;
		}
		{
			string info;
			get_device_info(CL_DEVICE_PROFILE, info);
			cout << "  CL_DEVICE_PROFILE                  : " << info << endl;
		}
		{
			cl_uint info;
			get_device_info(CL_DEVICE_MAX_CLOCK_FREQUENCY, info);
			cout << "  CL_DEVICE_MAX_CLOCK_FREQUENCY      : " << info << " MHz" << endl;
		}
		{
			cl_uint info;
			get_device_info(CL_DEVICE_MAX_COMPUTE_UNITS, info);
			cout << "  CL_DEVICE_MAX_COMPUTE_UNITS        : " << info << endl;
		}
		{
			cl_ulong info;
			get_device_info(CL_DEVICE_GLOBAL_MEM_SIZE, info);
			cout << "  CL_DEVICE_GLOBAL_MEM_SIZE          : " << static_cast<double>(info) / 1024 / 1024 << " MiB" << endl;
		}
		{
			cl_ulong info;
			get_device_info(CL_DEVICE_LOCAL_MEM_SIZE, info);
			cout << "  CL_DEVICE_LOCAL_MEM_SIZE           : " << static_cast<double>(info) / 1024 << " KiB" << endl;
		}
		{
			cl_ulong info;
			get_device_info(CL_DEVICE_MAX_MEM_ALLOC_SIZE, info);
			cout << "  CL_DEVICE_MAX_MEM_ALLOC_SIZE       : " << static_cast<double>(info) / 1024 / 1024 << " MiB" << endl;
		}
		{
			cl_ulong info;
			get_device_info(CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE, info);
			cout << "  CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE : " << static_cast<double>(info) / 1024 << " KiB" << endl;
		}
		{
			cl_ulong info;
			get_device_info(CL_DEVICE_GLOBAL_MEM_CACHE_SIZE, info);
			cout << "  CL_DEVICE_GLOBAL_MEM_CACHE_SIZE    : " << static_cast<double>(info) / 1024 << " KiB" << endl;
		}
		{
			size_t info;
			get_device_info(CL_DEVICE_MAX_WORK_GROUP_SIZE, info);
			cout << "  CL_DEVICE_MAX_WORK_GROUP_SIZE      : " << info << endl;
		}
	}

	return true;
}

template <typename T>
void Gpu::get_device_info(cl_device_info deviceInfo, T &value)
{
	clGetDeviceInfo(deviceId, deviceInfo, sizeof(T), &value, nullptr);
}

template <typename T>
void Gpu::get_device_info(cl_device_info deviceInfo, T* values, size_t count)
{
	clGetDeviceInfo(deviceId, deviceInfo, sizeof(T) * count, values, NULL);
}

template <>
void Gpu::get_device_info(cl_device_info deviceInfo, string &value)
{
    size_t size = 0;
    unique_ptr<char> data;


	clGetDeviceInfo(deviceId, deviceInfo, 0, nullptr, &size);
    data.reset(new char[size]);

	clGetDeviceInfo(deviceId, deviceInfo, size, data.get(), nullptr);

    value.assign(data.get(), data.get() + size);
}

bool Gpu::opencl_load_source(string &file)
{
	stringstream sourceBuffer;
	if ( ! vm->count("cache"))
	{
		sourceBuffer << "__constant uint timestamp = " << time(nullptr) << ";" << endl; // prevent caching
	}

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

	program = clCreateProgramWithSource(context, 1, &sourceCStr, nullptr, &err);
	if ( ! program || err != CL_SUCCESS)
	{
		errMsg = (boost::format("Failed to create compute program (err %1%)") % perror(err)).str();
		return false;
	}

	return true;
}

bool Gpu::opencl_load_ptx(string &file)
{
	stringstream sourceBuffer;

	// load requested kernel
	{
		fstream fs(file, ios::binary | ios::in);

		if ( ! fs.is_open() || ! fs.good())
		{
			errMsg = "Failed to load requested kernel";
			return false;
		}

		sourceBuffer << fs.rdbuf() << endl;
	}

	const string &sourceString = sourceBuffer.str();
	const unsigned char *sourceBin = reinterpret_cast<const unsigned char *>(sourceString.c_str());
	size_t length = sourceString.length();

	program = clCreateProgramWithBinary(context, 1, &deviceId, &length, &sourceBin, nullptr, &err);
	if ( ! program || err != CL_SUCCESS)
	{
		errMsg = (boost::format("Failed to load pre-compiled compute program (err %1%)") % perror(err)).str();
		return false;
	}

	return true;
}

bool Gpu::opencl_save_ptx(string &file)
{
	size_t binarySize;
	err = clGetProgramInfo(program, CL_PROGRAM_BINARY_SIZES, sizeof(binarySize), &binarySize, nullptr);
	if (err != CL_SUCCESS)
	{
		errMsg = (boost::format("Failed to get program binary size (err %1%)") % perror(err)).str();
		return false;
	}

	unique_ptr<char[]> binaryBuffer(new char[binarySize]);
	char *binaryPtr = binaryBuffer.get();

	fill(binaryPtr, binaryPtr + binarySize, '\0');

	err = clGetProgramInfo(program, CL_PROGRAM_BINARIES, 0, &binaryPtr, nullptr);

	if (err != CL_SUCCESS)
	{
		errMsg = (boost::format("Failed to get program binary (err %1%)") % perror(err)).str();
		return false;
	}

	boost::filesystem::path parent_path = boost::filesystem::path(file).parent_path();

	if (! parent_path.empty() && ! boost::filesystem::is_directory(parent_path))
	{
		boost::filesystem::create_directory(parent_path);
	}

	fstream fs(file, ios::trunc | ios::out);
	if ( ! fs.is_open() || ! fs.good())
	{
		errMsg = "Failed to open target file";
		errMsg = (boost::format("Failed to open target file (is_open %1%, good %2%)") % fs.is_open() % fs.good()).str();
		return false;
	}

	fs.write(binaryBuffer.get(), string(binaryBuffer.get()).find_last_of("}") + 1);
	return true;
}

bool Gpu::opencl_build(string &kernelName, string &options)
{
	cout << "Build options: " << options << endl;

	err = clBuildProgram(program, 0, nullptr, options.c_str(), nullptr, nullptr);
	size_t len;
	array<char, 1024 * 10> logBuffer; // error message buffer, 10 KiB

	err = clGetProgramBuildInfo(program, deviceId, CL_PROGRAM_BUILD_LOG, logBuffer.size(), logBuffer.data(), &len);

	// trim buffer
	errMsg = logBuffer.data();
	boost::algorithm::trim(errMsg);

	if (errMsg.size() > 0)
	{
		cout << "Build log:" << endl << errMsg << endl;
	}

	if (err != CL_SUCCESS)
	{
		return false;
	}

	if (vm->count("save-ptx"))
	{
		string file = (*vm)["save-ptx"].as<string>();

		if (opencl_save_ptx(file))
		{
			cout << "Compiled program saved to: " << file << endl;
		}
		else
		{
			clog << "Warning: Compiled program could not be saved" << endl;
			clog << "         " << errMsg << endl;
			errMsg = "";
		}
	}

	kernel = clCreateKernel(program, kernelName.c_str(), &err);
	if ( ! kernel || err != CL_SUCCESS) {
		errMsg = (boost::format("Failed to create compute kernel \"%1%\" (err %2%)") % kernelName % perror(err)).str();
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

	if (vm->count("load-ptx"))
	{
		string file = (*vm)["load-ptx"].as<string>();
		cout << "Loading pre-compiled program: " << file << endl;

		if ( ! opencl_load_ptx(file))
		{
			clog << "Warning: could not build program from pre-compiled files, building from source" << endl;
		}
	}

	if ( ! program && ! opencl_load_source(kernelName))
	{
		return false;
	}

	stringstream options;
	options << "-D NROUNDS=" << NROUNDS(keyLength_);

	if (vm->count("verbose"))
	{
		options << " -cl-nv-verbose";
	}


	if (vm->count("unsafe-optimizations"))
	{
		options << " -cl-unsafe-math-optimizations";
	}

	if ( ! opencl_build(kernelName, options.str()))
	{
		return false;
	}

	local = (*vm)["work-items"].as<size_t>();

	if (local == 0)
	{
		clog << "  Forcing at least 1 work-item per work-group" << endl;
	}

	if (kernelName.find("local") != string::npos && local != 256)
	{
		clog << "  Forcing 256 work-items per work-group" << endl;
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

	if (sampleLengthPadded > 0 && sampleLengthPadded % lengthPad != 0)
	{
		sampleLengthPadded += lengthPad - (sampleLengthPadded % lengthPad);
	}

	global = sampleLengthPadded / 16;

	cout << "Sample size padded: " << sampleLengthPadded << " bytes (padding to " << lengthPad << " bytes)" << endl;

	cl_ulong max_alloc_size;
	err = clGetDeviceInfo(deviceId, CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(max_alloc_size), &max_alloc_size, nullptr);
	if (err != CL_SUCCESS)
	{
		errMsg = (boost::format("Failed to get maximum allocable memory size (err %1%)") % perror(err)).str();
		return false;
	}

	if (sampleLengthPadded + RKLENGTH(256) > max_alloc_size)
	{
		errMsg = (boost::format("Required memory is too big (%1% bytes), maximum size is %2% bytes") % (sampleLengthPadded + RKLENGTH(256)) % max_alloc_size).str();
		return false;
	}

	memState = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR, sampleLengthPadded, nullptr, &err);
	if (err != CL_SUCCESS || memState == nullptr)
	{
		errMsg = (boost::format("Failed to allocate device memory (state, size: %1%; %2%)") % sampleLengthPadded % perror(err)).str();
		return false;
	}

	memRoundKeys = clCreateBuffer(context, CL_MEM_READ_ONLY, RKLENGTH(256), nullptr, &err);
	if (err != CL_SUCCESS || memRoundKeys == nullptr)
	{
		errMsg = (boost::format("Failed to allocate device memory (roundkeys, size: %1%; %2%)") % RKLENGTH(256) % perror(err)).str();
		return false;
	}

	err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &memState);
	if (err != CL_SUCCESS)
	{
		errMsg = (boost::format("Failed to bind kernel argument \"memState\" (%1%)") % perror(err)).str();
		return false;
	}

	err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &memRoundKeys);
	if (err != CL_SUCCESS)
	{
		errMsg = (boost::format("Failed to bind kernel argument \"memRoundKeys\" (%1%)") % perror(err)).str();
		return false;
	}

	unsigned char key[KEYLENGTH(256)] = { 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32 };

	rijndaelSetupEncrypt(reinterpret_cast<unsigned long *>(roundKeys), key, keyLength);

	err = clEnqueueWriteBuffer(queue, memRoundKeys, CL_TRUE, 0, RKLENGTH(keyLength), roundKeys, 0, nullptr, nullptr);
	if (err != CL_SUCCESS) {
		errMsg = (boost::format("Failed write data to buffer \"roundkeys\" (%1%)") % perror(err)).str();
		return false;
	}

	return true;
}

std::string Gpu::run(Bench::Container &sample)
{
	unique_ptr<unsigned char> outBuffer(new unsigned char[sample.length * sizeof(sample.data[0])]);

	unsigned int nrounds = NROUNDS(keyLength);
	assert(nrounds == 10 || nrounds == 12 || nrounds == 14);

	auto start = hrc::now();

	err = clEnqueueWriteBuffer(queue, memState, CL_FALSE, 0, sample.length, sample.data, 0, nullptr, nullptr);
	if (err != CL_SUCCESS) {
		errMsg = "Failed write data to buffer (state)";
		return "";
	}

	err = clEnqueueNDRangeKernel(queue, kernel, 1, nullptr, &global, &local, 0, nullptr, nullptr);
	if (err != CL_SUCCESS) {
		errMsg = (boost::format("Failed to execute kernel (%1%)") % perror(err)).str();
		return "";
	}

	err = clEnqueueReadBuffer(queue, memState, CL_FALSE, 0, sample.length, sample.data, 0, nullptr, nullptr);
	if (err != CL_SUCCESS) {
		errMsg = (boost::format("Failed to read from buffer (%1%)") % perror(err)).str();
		return "";
	}

	err = clFinish(queue);
	if (err != CL_SUCCESS) {
		errMsg = (boost::format("Failed to finish queue (%1%)") % perror(err)).str();
		return "";
	}

	auto end = hrc::now();
	return boost::lexical_cast<string>(ch::duration_cast<ch::microseconds>(end - start).count());
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

string Gpu::pparam_name(cl_device_info paramName, bool trim)
{
	static array<string, 62> names = {{
		"CL_DEVICE_TYPE                         ",
		"CL_DEVICE_VENDOR_ID                    ",
		"CL_DEVICE_MAX_COMPUTE_UNITS            ",
		"CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS     ",
		"CL_DEVICE_MAX_WORK_GROUP_SIZE          ",
		"CL_DEVICE_MAX_WORK_ITEM_SIZES          ",
		"CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR  ",
		"CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT ",
		"CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT   ",
		"CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG  ",
		"CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT ",
		"CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE",
		"CL_DEVICE_MAX_CLOCK_FREQUENCY          ",
		"CL_DEVICE_ADDRESS_BITS                 ",
		"CL_DEVICE_MAX_READ_IMAGE_ARGS          ",
		"CL_DEVICE_MAX_WRITE_IMAGE_ARGS         ",
		"CL_DEVICE_MAX_MEM_ALLOC_SIZE           ",
		"CL_DEVICE_IMAGE2D_MAX_WIDTH            ",
		"CL_DEVICE_IMAGE2D_MAX_HEIGHT           ",
		"CL_DEVICE_IMAGE3D_MAX_WIDTH            ",
		"CL_DEVICE_IMAGE3D_MAX_HEIGHT           ",
		"CL_DEVICE_IMAGE3D_MAX_DEPTH            ",
		"CL_DEVICE_IMAGE_SUPPORT                ",
		"CL_DEVICE_MAX_PARAMETER_SIZE           ",
		"CL_DEVICE_MAX_SAMPLERS                 ",
		"CL_DEVICE_MEM_BASE_ADDR_ALIGN          ",
		"CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE     ",
		"CL_DEVICE_SINGLE_FP_CONFIG             ",
		"CL_DEVICE_GLOBAL_MEM_CACHE_TYPE        ",
		"CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE    ",
		"CL_DEVICE_GLOBAL_MEM_CACHE_SIZE        ",
		"CL_DEVICE_GLOBAL_MEM_SIZE              ",
		"CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE     ",
		"CL_DEVICE_MAX_CONSTANT_ARGS            ",
		"CL_DEVICE_LOCAL_MEM_TYPE               ",
		"CL_DEVICE_LOCAL_MEM_SIZE               ",
		"CL_DEVICE_ERROR_CORRECTION_SUPPORT     ",
		"CL_DEVICE_PROFILING_TIMER_RESOLUTION   ",
		"CL_DEVICE_ENDIAN_LITTLE                ",
		"CL_DEVICE_AVAILABLE                    ",
		"CL_DEVICE_COMPILER_AVAILABLE           ",
		"CL_DEVICE_EXECUTION_CAPABILITIES       ",
		"CL_DEVICE_QUEUE_PROPERTIES             ",
		"CL_DEVICE_NAME                         ",
		"CL_DEVICE_VENDOR                       ",
		"CL_DRIVER_VERSION                      ",
		"CL_DEVICE_PROFILE                      ",
		"CL_DEVICE_VERSION                      ",
		"CL_DEVICE_EXTENSIONS                   ",
		"CL_DEVICE_PLATFORM                     ",
		"CL_DEVICE_DOUBLE_FP_CONFIG             ",
		"CL_DEVICE_HALF_FP_CONFIG               ",
		"CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF  ",
		"CL_DEVICE_HOST_UNIFIED_MEMORY          ",
		"CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR     ",
		"CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT    ",
		"CL_DEVICE_NATIVE_VECTOR_WIDTH_INT      ",
		"CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG     ",
		"CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT    ",
		"CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE   ",
		"CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF     ",
		"CL_DEVICE_OPENCL_C_VERSION             "
	}};

	size_t id = paramName - 0x1000;

	if (id >=0 && id < names.size())
	{
		string name = names[id];
		if (trim)
		{
			boost::algorithm::trim(name);
		}

		return name;
	}

	return (boost::format("Unknown param name (%1%)") % paramName).str();
}

string Gpu::perror(cl_int err, bool trim)
{
	static array<string, 50> errors = {{
		"CL_SUCCESS                                  ", //  0 
		"CL_DEVICE_NOT_FOUND                         ", // -1
		"CL_DEVICE_NOT_AVAILABLE                     ", // -2
		"CL_COMPILER_NOT_AVAILABLE                   ", // -3
		"CL_MEM_OBJECT_ALLOCATION_FAILURE            ", // -4
		"CL_OUT_OF_RESOURCES                         ", // -5
		"CL_OUT_OF_HOST_MEMORY                       ", // -6
		"CL_PROFILING_INFO_NOT_AVAILABLE             ", // -7
		"CL_MEM_COPY_OVERLAP                         ", // -8
		"CL_IMAGE_FORMAT_MISMATCH                    ", // -9
		"CL_IMAGE_FORMAT_NOT_SUPPORTED               ", // -10
		"CL_BUILD_PROGRAM_FAILURE                    ", // -11
		"CL_MAP_FAILURE                              ", // -12
		"CL_MISALIGNED_SUB_BUFFER_OFFSET             ", // -13
		"CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST", // -14
		"CL_INVALID_VALUE                            ", // -30
		"CL_INVALID_DEVICE_TYPE                      ", // -31
		"CL_INVALID_PLATFORM                         ", // -32
		"CL_INVALID_DEVICE                           ", // -33
		"CL_INVALID_CONTEXT                          ", // -34
		"CL_INVALID_QUEUE_PROPERTIES                 ", // -35
		"CL_INVALID_COMMAND_QUEUE                    ", // -36
		"CL_INVALID_HOST_PTR                         ", // -37
		"CL_INVALID_MEM_OBJECT                       ", // -38
		"CL_INVALID_IMAGE_FORMAT_DESCRIPTOR          ", // -39
		"CL_INVALID_IMAGE_SIZE                       ", // -40
		"CL_INVALID_SAMPLER                          ", // -41
		"CL_INVALID_BINARY                           ", // -42
		"CL_INVALID_BUILD_OPTIONS                    ", // -43
		"CL_INVALID_PROGRAM                          ", // -44
		"CL_INVALID_PROGRAM_EXECUTABLE               ", // -45
		"CL_INVALID_KERNEL_NAME                      ", // -46
		"CL_INVALID_KERNEL_DEFINITION                ", // -47
		"CL_INVALID_KERNEL                           ", // -48
		"CL_INVALID_ARG_INDEX                        ", // -49
		"CL_INVALID_ARG_VALUE                        ", // -50
		"CL_INVALID_ARG_SIZE                         ", // -51
		"CL_INVALID_KERNEL_ARGS                      ", // -52
		"CL_INVALID_WORK_DIMENSION                   ", // -53
		"CL_INVALID_WORK_GROUP_SIZE                  ", // -54
		"CL_INVALID_WORK_ITEM_SIZE                   ", // -55
		"CL_INVALID_GLOBAL_OFFSET                    ", // -56
		"CL_INVALID_EVENT_WAIT_LIST                  ", // -57
		"CL_INVALID_EVENT                            ", // -58
		"CL_INVALID_OPERATION                        ", // -59
		"CL_INVALID_GL_OBJECT                        ", // -60
		"CL_INVALID_BUFFER_SIZE                      ", // -61
		"CL_INVALID_MIP_LEVEL                        ", // -62
		"CL_INVALID_GLOBAL_WORK_SIZE                 ", // -63
		"CL_INVALID_PROPERTY                         ", // -64
	}};

	size_t id = -err;

	if (id >= 15 && id < 30)
	{
		return (boost::format("Unknown error (%1%)") % err).str();
	}

	if (id >= 30)
	{
		id -= 15;
	}

	if (id < errors.size())
	{
		string error = errors[id];
		if (trim)
		{
			boost::algorithm::trim(error);
		}

		return error;
	}

	return (boost::format("Unknown error (%1%)") % err).str();
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