#include "stdafx.h"
#include "cl_encrypt.h"
#include "key_schedule.h"
#include <cstdlib>
#include <fstream>
#include <sstream>

static cl_int err = 0;
static cl_platform_id platform_id = nullptr;
static cl_device_id device_id = nullptr;
static cl_context context = nullptr;
static cl_command_queue queue = nullptr;
static cl_program program = nullptr;
static cl_kernel kernel = nullptr;
static cl_mem state = nullptr;
static cl_mem roundkeys = nullptr;
static char *source = "./aes-encrypt.cl";

void cl_print_error(cl_int err)
{
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
}

void cl_build_program()
{
	err = clGetPlatformIDs(1, &platform_id, NULL);
	if (err != CL_SUCCESS) {
		printf("Error: could not connect to compute device\n");
		cl_print_error(err);
		exit(EXIT_FAILURE);
	}

	device_id = NULL;
	// Get a device of the appropriate type
	err = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 1, &device_id, NULL);

	if (err != CL_SUCCESS) {
		printf("Warning: no GPU support, falling back to CPU\n");

		err = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_CPU, 1, &device_id,
		                     NULL);
	}

	if ( ! device_id) {
		printf("Error: could not get device id\n");
		cl_print_error(err);
		exit(EXIT_FAILURE);
	}

	// Create a compute context
	context = clCreateContext(0, 1, &device_id, NULL, NULL, &err);
	if ( ! context) {
		printf("Error: failed to create a compute context\n");
		cl_print_error(err);
		exit(EXIT_FAILURE);
	}

	// Create a command queue
	queue = clCreateCommandQueue(context, device_id, 0, &err);
	if ( ! queue) {
		printf("Error: failed to create a command queue\n");
		cl_print_error(err);
		exit(EXIT_FAILURE);
	}

	std::fstream f(source, std::ios::in);

	if ( ! f.is_open())
	{
		printf("Error: failed to open %s\n", source);
		exit(EXIT_FAILURE);
	}

	f.seekg (0, std::ios::end);
	size_t size = (size_t) f.tellg();
	f.seekg (0, std::ios::beg);

	if (size == 0)
	{
		printf("Error: %s is empty\n", source);
		exit(EXIT_FAILURE);
	}

	char *source_str = new char[size];

	f.read(source_str, size);

	program = clCreateProgramWithSource(context, 1, (const char **) &source_str,
	                                    NULL, &err);
	if ( ! program) {
		printf("Error: failed to create compute program\n");
		cl_print_error(err);
		exit(EXIT_FAILURE);
	}

	err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
	if (err != CL_SUCCESS) {
		size_t len;
		char buffer[1024 * 10]; // error message buffer, 10 KiB

		clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG,
		                      sizeof(buffer), buffer, &len);

		printf("Error: failed to create program executable\n%s\n", buffer);
		cl_print_error(err);
		exit(EXIT_FAILURE);
	}
}

void cl_init(char *kernel_name, uint8_t *state_ptr,unsigned int state_length,
	         unsigned int roundkeys_length)
{
	if (program == nullptr)
	{
		cl_build_program();
	}

	// Create the compute kernel in the program
	kernel = clCreateKernel(program, kernel_name, &err);
	if ( ! kernel || err != CL_SUCCESS) {
		printf("Error: failed to create compute kernel\n");
		cl_print_error(err);
		exit(EXIT_FAILURE);
	}

	// Create buffers
	state = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR, 
		                   state_length, NULL, &err);
	if (err != CL_SUCCESS)
	{
		printf("Error: failed to allocate device memory (state, size: %lu)\n",
		       state_length);
		cl_print_error(err);
		exit(EXIT_FAILURE);
	}
	
	roundkeys = clCreateBuffer(context, CL_MEM_READ_ONLY, roundkeys_length, NULL, 
		                       &err);
	if (err != CL_SUCCESS)
	{
		printf("Error: failed to allocate device memory (roundkeys, size: %lu)\n",
		       roundkeys_length);
		cl_print_error(err);
		exit(EXIT_FAILURE);
	}

	// Bind buffers
	err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &state);
	if (err != CL_SUCCESS)
	{
		printf("Error: failed to bind kernel argument \"buffer\" [#0]\n");
		cl_print_error(err);
		exit(EXIT_FAILURE);
	}

	err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &roundkeys);
	if (err != CL_SUCCESS)
	{
		printf("Error: failed to bind kernel argument \"roundkeys\" [#1]\n");
		cl_print_error(err);
		exit(EXIT_FAILURE);
	}
}

void cl_encrypt(uint8_t *roundkeys, unsigned int rk_length, uint8_t *text,
	            unsigned int text_length)
{
	
}

void cl_release_kernel()
{
	if (state != nullptr)
	{
		clReleaseMemObject(state);
		state = nullptr;
	}
	
	if (roundkeys != nullptr)
	{
		clReleaseMemObject(roundkeys);
		roundkeys = nullptr;
	}

	if (kernel != nullptr)
	{
		clReleaseKernel(kernel);
		kernel = nullptr;
	}
}

void cl_release_all()
{
	cl_release_kernel();

	if (program != nullptr)
	{
		clReleaseProgram(program);
		program = nullptr;
	}

	if (queue != nullptr)
	{
		clReleaseCommandQueue(queue);
		queue = nullptr;
	}

	if (context != nullptr)
	{
		clReleaseContext(context);
		context = nullptr;

	}
}