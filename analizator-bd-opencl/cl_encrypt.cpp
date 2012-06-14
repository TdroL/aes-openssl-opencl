#include "stdafx.h"
#include <CL/cl.h>
#include "cl_encrypt.h"
#include "md5.h"
#include <cstdlib>
#include <cstdarg>
#include <fstream>
#include <sstream>
#include <memory> 
#include <ctime>
using namespace std;

static cl_int err = 0;
static cl_platform_id platform_id = nullptr;
static cl_device_id device_id = nullptr;
static cl_context context = nullptr;
static cl_command_queue queue = nullptr;
static cl_program program = nullptr;
static cl_kernel kernel = nullptr;
static cl_mem mem_state = nullptr;
static cl_mem mem_roundkeys = nullptr;
static size_t state_length;
static size_t state_length_padded;
static size_t roundkeys_length;
#if defined(DEBUG) || defined(_DEBUG)
	static char *source_path = "C:/Users/tdrol/Documents/Visual Studio 2010/Projects/analizator/Release/kernels/aes_encrypt.cl";
#else
	static char *source_path = "./kernels/aes_encrypt.cl";
#endif

void cl_print_error_silent(cl_int err, const char *msg, ...) {};
void cl_print_error(cl_int err, const char *msg, ...)
{
    va_list argptr;
	va_start(argptr, msg);
    vfprintf(stderr, msg, argptr);
    va_end(argptr);

	switch (err)
	{
		case CL_INVALID_ARG_INDEX: printf("- Invalid argument index\n"); break;
		case CL_INVALID_ARG_SIZE: printf(" - Invalid argument data size\n"); break;
		case CL_INVALID_ARG_VALUE: printf("- Invalid argument value\n"); break;
		case CL_INVALID_COMMAND_QUEUE: printf("- Invalid command-queue\n"); break;
		case CL_INVALID_CONTEXT: printf("- Invalid context or buffer\n"); break;
		case CL_INVALID_EVENT_WAIT_LIST: printf("- Invalid event-wait list\n"); break;
		case CL_INVALID_GLOBAL_OFFSET: printf(" - global_work_offset is not NULL\n"); break;
		case CL_INVALID_KERNEL: printf(" - Invalid kernel object\n"); break;
		case CL_INVALID_KERNEL_ARGS: printf(" - Kernel argument values have not been specified\n"); break;
		case CL_INVALID_MEM_OBJECT: printf("- Invalid buffer\n"); break;
		case CL_INVALID_PROGRAM_EXECUTABLE: printf(" - No valid program avaible\n"); break;
		case CL_INVALID_SAMPLER: printf("- Invalid sampler object\n"); break;
		case CL_INVALID_VALUE: printf("- Offset or size out-of-bounds or *results is a NULL value\n"); break;
		case CL_INVALID_WORK_DIMENSION: printf(" - work_dim is not a valid value\n"); break;
		case CL_INVALID_WORK_GROUP_SIZE: printf(" - global_work_size is not evenly divisable by local_work_size\n"); break;
		case CL_INVALID_WORK_ITEM_SIZE: printf(" - local_work_size is greater than CL_DEVICE_MAX_WORK_ITEM_SIZES\n"); break;
		case CL_MEM_OBJECT_ALLOCATION_FAILURE: printf("- Failed to allocate or read memory for data store associated with buffer\n"); break;
		case CL_OUT_OF_HOST_MEMORY: printf("- Failed to allocate or read resources required by the OpenCL implementation on the host\n"); break;
		case CL_OUT_OF_RESOURCES: printf(" - Nobody knows what happened (CL_OUT_OF_RESOURCES)\n"); break;
	}

	exit(EXIT_FAILURE);
}

char * file_get_contents(const char *file_path, int *ret_size = nullptr)
{
	fstream f(file_path, ios::in | ios::ate);
	f.clear();

	if (ret_size != nullptr)
	{
		*ret_size = 0;
	}

	if ( ! f.is_open())
	{
		return nullptr;
	}

	int size = (int) f.tellg();
	f.seekg (0, ios::beg);

	if (size <= 0)
	{
		return nullptr;
	}

	char *content = new char[size];
	memset(content, 0, size);

	f.read(content, size);

	if (ret_size != nullptr)
	{
		*ret_size = size;
	}

	return content;
}

bool file_put_contents(const char *file_path, char *data, streamsize size = -1)
{
	fstream f(file_path, ios::out | ios::trunc);
	f.open(file_path);
	f.clear();

	if ( ! f.is_open())
	{
		return false;
	}

	if (size < streamsize(-1))
	{
		size = strlen(data);
	}

	f.write(data, size);

	f.close();

	return true;
}

bool cl_build_program(bool silent = false)
{
	auto cl_print_error = ::cl_print_error;
	if (silent)
	{
		cl_print_error = cl_print_error_silent;
	}

	err = clGetPlatformIDs(1, &platform_id, NULL);
	if (err != CL_SUCCESS) {
		cl_print_error(err, "Error: could not connect to compute device\n");
		return false;
	}

	// Get a device of the appropriate type
	err = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 1, &device_id, NULL);

	if (err != CL_SUCCESS) {
		printf("Warning: no GPU support, falling back to CPU\n");

		err = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_CPU, 1, &device_id,
		                     NULL);
	}

	if ( ! device_id) {
		cl_print_error(err, "Error: could not get device id\n");
		return false;
	}

	// Create a compute context
	context = clCreateContext(0, 1, &device_id, NULL, NULL, &err);
	if ( ! context || err != CL_SUCCESS) {
		cl_print_error(err, "Error: failed to create a compute context\n");
		return false;
	}

	// Create a command queue
	queue = clCreateCommandQueue(context, device_id, 0, &err);
	if ( ! queue || err != CL_SUCCESS) {
		cl_print_error(err, "Error: failed to create a command queue\n");
		return false;
	}

	int source_size = 0;
	char *source_str = nullptr;
	bool cache_program = true;
	string cached_source_path = source_path;
	cached_source_path += ".cached";

#ifdef ENABLE_CACHING
	source_str = file_get_contents(cached_source_path.c_str(), &source_size);

	if (source_str != nullptr && source_size != 0)
	{
		program = clCreateProgramWithBinary(context, 1, &device_id, (const size_t *) &source_size, (const unsigned char **) &source_str, NULL, &err);

		if ( ! program || err != CL_SUCCESS)
		{
			printf("Warning: failed to create compute program from binary\n");
			program = nullptr;
		}
		else
		{
			printf("Info: program loaded from cached binary\n");
			cache_program = false;
		}

		delete[] source_str;
	}
#endif

	if (program == nullptr)
	{
		source_str = file_get_contents(source_path, &source_size);

		if (source_str == nullptr)
		{
			cl_print_error(NULL, "Error: failed to load source program from %s\n", source_path);
		}
		
		char footer_str[64];
		sprintf_s(footer_str, 64, "\n// time() = %u", time(NULL));

		string source_footer;
		source_footer.reserve(source_size + 64);
		source_footer.append(source_str).append(footer_str);

		const char *source_ptr = source_footer.c_str();

		program = clCreateProgramWithSource(context, 1, &source_ptr, NULL, &err);

		delete[] source_str;
	}
	
	if ( ! program || err != CL_SUCCESS)
	{
		cl_print_error(err, "Error: failed to create compute program\n");
		return false;
	}

	const char *options = "-cl-mad-enable -cl-unsafe-math-optimizations";

	err = clBuildProgram(program, 0, NULL, options, NULL, NULL);
	size_t len;
	char buffer[1024 * 10]; // error message buffer, 10 KiB
	char *buffer_ptr = buffer;

	clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);

	// trim buffer
	while (buffer_ptr[0] == ' ' 
		|| buffer_ptr[0] == '\n' 
		|| buffer_ptr[0] == '\t' 
		|| buffer_ptr[0] == '\r')
	{
		len--;
		buffer_ptr += 1;
	}

	if (len > 1)
	{
		printf("Build log:\n%s\n", buffer_ptr);
	}

	if (err != CL_SUCCESS)
	{
		cl_print_error(err, "Error: failed to create program executable\n");
		return false;
	}

#ifdef ENABLE_CACHING
	if ( ! cache_program)
	{
		return true;
	}

	size_t binary_size;
	err = clGetProgramInfo(program, CL_PROGRAM_BINARY_SIZES, sizeof(binary_size), &binary_size, NULL);
	if (err != CL_SUCCESS)
	{
		printf("Warning: failed to retrive binary size\n");
		return true;
	}

	char *binary_str = new char[binary_size];
	memset(binary_str, 0, binary_size);
	err = clGetProgramInfo(program, CL_PROGRAM_BINARIES, sizeof(size_t), &binary_str, NULL);
	if (err != CL_SUCCESS)
	{
		printf("Warning: failed to retrive binaries\n");
		return true;
	}

	file_put_contents(cached_source_path.c_str(), binary_str, binary_size);

    delete[] binary_str;
#endif

	return true;
}

bool cl_init(char *kernel_name, size_t state_length, size_t roundkeys_length, bool silent)
{
	if (program == nullptr)
	{
		if ( ! cl_build_program(silent))
		{
			return false;
		}
	}
	else
	{
		cl_release_kernel();
	}

	auto cl_print_error = ::cl_print_error;
	if (silent)
	{
		cl_print_error = cl_print_error_silent;
	}

	// Create the compute kernel in the program
	kernel = clCreateKernel(program, kernel_name, &err);
	if ( ! kernel || err != CL_SUCCESS) {
		cl_print_error(err, "Error: failed to create compute kernel (%s)\n", kernel_name);
		return false;
	}

	// Create buffers
	if (state_length < 16) // 128 bit / 8 = 16 bytes
	{
		cl_print_error(err, "Error: state length too small, must be at least 16 bytes (128 bits) long\n");
		return false;
	}

	state_length_padded = ((size_t) (state_length / 16 + 256 - 1) / 256) * 256 * 16;

	mem_state = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR, 
							   state_length_padded, NULL, &err);
	if (err != CL_SUCCESS)
	{
		cl_print_error(err,  "Error: failed to allocate device memory " \
					   "(state, size: %lu)\n", state_length);
		return false;
	}
	::state_length = state_length;

	mem_roundkeys = clCreateBuffer(context, CL_MEM_READ_ONLY, roundkeys_length,
								   NULL, &err);
	if (err != CL_SUCCESS)
	{
		cl_print_error(err, "Error: failed to allocate device memory " \
					   "(roundkeys, size: %lu)\n", roundkeys_length);
		return false;
	}
	::roundkeys_length = roundkeys_length;

	// Bind buffers
	err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &mem_state);
	if (err != CL_SUCCESS)
	{
		cl_print_error(err, "Error: failed to bind kernel argument " \
					   "\"mem_state\" [#0]\n");
		return false;
	}

	err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &mem_roundkeys);
	if (err != CL_SUCCESS)
	{
		cl_print_error(err, "Error: failed to bind kernel argument " \
					   "\"mem_roundkeys\" [#1]\n");
		return false;
	}

	return true;
}

void cl_print_platform_info()
{
	printf("\nPlatform info:\n");
	const size_t param_value_size = 1024*10;
	char param_value[param_value_size];

	clGetPlatformInfo(platform_id, CL_PLATFORM_PROFILE,
  						param_value_size, param_value, NULL);
	printf("  platform profile     %s\n", param_value);

	clGetPlatformInfo(platform_id, CL_PLATFORM_VERSION,
  						param_value_size, param_value, NULL);
	printf("  platform version     %s\n", param_value);

	clGetPlatformInfo(platform_id, CL_PLATFORM_NAME,
  						param_value_size, param_value, NULL);
	printf("  platform name        %s\n", param_value);

	clGetPlatformInfo(platform_id, CL_PLATFORM_VENDOR ,
  						param_value_size, param_value, NULL);
	printf("  platform vendor      %s\n", param_value);

	int offset = 0;
	char *pos;
	size_t size;
	clGetPlatformInfo(platform_id, CL_PLATFORM_EXTENSIONS,
  						param_value_size, param_value, &size);

	pos = strstr(param_value + offset, " ");
	if (pos != NULL && size > 0)
	{
		*pos = '\0';
		
		printf("  platform extensions  %s\n", param_value);
		offset = pos - param_value + 1;

		while ((pos = strstr(param_value + offset, " ")) && pos < (param_value + size - 1))
		{
			*pos = '\0';

			printf("                       %s\n", param_value + offset);

			offset = pos - param_value + 1;
		}
	}
	else
	{
		printf("  platform extensions  (none)\n");
	}
}

void cl_print_device_info()
{
	printf("\nDevice info:\n");
	unsigned long int param_value;
	char param_string[1024];

	clGetDeviceInfo(device_id, CL_DEVICE_NAME, sizeof(param_string) * 1024, &param_string, NULL);
	printf("  device name              %s\n", param_string);

	clGetDeviceInfo(device_id, CL_DEVICE_VERSION, sizeof(param_string) * 1024, &param_string, NULL);
	printf("  device version           %s\n", param_string);

	clGetDeviceInfo(device_id, CL_DRIVER_VERSION, sizeof(param_string) * 1024, &param_string, NULL);
	printf("  driver version           %s\n", param_string);

	param_value = clGetDeviceInfo(device_id, CL_DEVICE_LOCAL_MEM_SIZE, sizeof(param_value), &param_value, NULL);
	{
		double memory = (double) param_value;
		char units[][4] = {"B  ", "KiB", "MiB", "GiB"};
		char *unit = units[0];
		for (int i = 1; memory >= 1024; i++)
		{
			memory /= 1024;
			unit = units[i];
		}
		printf("  local memory size        %.2f %s\n", memory, unit);
	}

	clGetDeviceInfo(device_id, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(param_value), &param_value, NULL);
	{
		double memory = (double) param_value;
		char units[][4] = {"B  ", "KiB", "MiB", "GiB"};
		char *unit = units[0];
		for (int i = 1; memory >= 1024; i++)
		{
			memory /= 1024;
			unit = units[i];
		}
		printf("  global memory size       %.2f %s\n", memory, unit);
	}

	clGetDeviceInfo(device_id, CL_DEVICE_GLOBAL_MEM_CACHE_SIZE, sizeof(param_value), &param_value, NULL);
	{
		double memory = (double) param_value;
		char units[][4] = {"B  ", "KiB", "MiB", "GiB"};
		char *unit = units[0];
		for (int i = 1; memory >= 1024; i++)
		{
			memory /= 1024;
			unit = units[i];
		}
		printf("  global mem cache size    %.2f %s\n", memory, unit);
	}

	clGetDeviceInfo(device_id, CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(param_value), &param_value, NULL);
	{
		double memory = (double) param_value;
		char units[][4] = {"B  ", "KiB", "MiB", "GiB"};
		char *unit = units[0];
		for (int i = 1; memory >= 1024; i++)
		{
			memory /= 1024;
			unit = units[i];
		}
		printf("  maximum avaible mem      %.2f %s\n", memory, unit);
	}

	clGetDeviceInfo(device_id, CL_DEVICE_ADDRESS_BITS, sizeof(param_value), &param_value, NULL);
	printf("  address bits             %u\n", param_value);

	clGetDeviceInfo(device_id, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(param_value), &param_value, NULL);
	printf("  maximum compute units    %u\n", param_value);

	clGetDeviceInfo(device_id, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(param_value), &param_value, NULL);
	printf("  maximum work-group size  %u\n", param_value);

	clGetDeviceInfo(device_id, CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(param_value), &param_value, NULL);
	printf("  maximum work-item sizes  %u\n", param_value);

	clGetDeviceInfo(device_id, CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(param_value), &param_value, NULL);
	printf("  max clock frequency      %u MHz\n", param_value);
}

void cl_update(uint8_t *text, uint32_t *roundkeys)
{
	size_t local = 256;
	size_t global = state_length_padded / 16;

	err = clEnqueueWriteBuffer(queue, mem_roundkeys, CL_FALSE, 0, roundkeys_length, 
							   roundkeys, 0, NULL, NULL);
	if (err != CL_SUCCESS) {
		cl_print_error(err, "Error: failed write data to buffer (roundkeys)\n");
	}

	err = clEnqueueWriteBuffer(queue, mem_state, CL_FALSE, 0, state_length, text, 
							   0, NULL, NULL);
	if (err != CL_SUCCESS) {
		cl_print_error(err, "Error: failed write data to buffer (state)\n");
	}

	err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global, &local, 0, NULL, NULL);
	if (err != CL_SUCCESS) {
		cl_print_error(err, "Error: failed to execute kernel\n");
	}

	err = clEnqueueReadBuffer(queue, mem_state, CL_TRUE, 0, state_length, text,
							  0, NULL, NULL);
	if (err != CL_SUCCESS) {
		cl_print_error(err, "Error: failed to read from buffer\n");
	}

	clFinish(queue);
}

void cl_release_kernel()
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
}

void cl_release_all()
{
	cl_release_kernel();

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
}