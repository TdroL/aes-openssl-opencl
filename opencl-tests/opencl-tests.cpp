#include "stdafx.h"
#include <cstdint>
#include <cstdlib>
#include <cstdarg>
#include <CL/cl.h>

static cl_int err = 0;
static cl_platform_id platform_id = nullptr;
static cl_device_id device_id = nullptr;
static cl_context context = nullptr;
static cl_command_queue queue = nullptr;
static cl_program program = nullptr;
static cl_kernel kernel = nullptr;
static cl_mem mem_state_pinned = nullptr;
static cl_mem mem_state = nullptr;
static cl_mem mem_roundkeys = nullptr;

void cl_print_error(cl_int err, const char *msg, ...);
void cl_release_kernel();
void cl_release_all();

int main(int argc, _TCHAR* argv[])
{
	err = clGetPlatformIDs(1, &platform_id, NULL);
	if (err != CL_SUCCESS) {
		cl_print_error(err, "Error: could not connect to compute device\n");
	}

	// Get a device of the appropriate type
	err = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 1, &device_id, NULL);

	if (err != CL_SUCCESS) {
		printf("Warning: no GPU support, falling back to CPU\n");

		err = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_CPU, 1, &device_id, NULL);
	}

	if ( ! device_id) {
		cl_print_error(err, "Error: could not get device id\n");
	}

	// Create a compute context
	context = clCreateContext(0, 1, &device_id, NULL, NULL, &err);
	if ( ! context || err != CL_SUCCESS) {
		cl_print_error(err, "Error: failed to create a compute context\n");
	}

	// Create a command queue
	queue = clCreateCommandQueue(context, device_id, 0, &err);
	if ( ! queue || err != CL_SUCCESS) {
		cl_print_error(err, "Error: failed to create a command queue\n");
	}

	const char *source_str = "__kernel void main(__global uint4 *state) { \n" \
							 "  const uint id = get_global_id(0); \n" \
							 "  state[id] += 1; \n" \
							 "}";

	program = clCreateProgramWithSource(context, 1, &source_str, NULL, &err);
	
	if ( ! program || err != CL_SUCCESS) {
		cl_print_error(err, "Error: failed to create compute program\n");
	}

	err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
	if (err != CL_SUCCESS) {
		size_t len;
		char buffer[1024 * 10]; // error message buffer, 10 KiB

		clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);

		cl_print_error(err, "Error: failed to create program executable\n%s\n",
					   buffer);
	}

	kernel = clCreateKernel(program, "main", &err);
	if ( ! kernel || err != CL_SUCCESS) {
		cl_print_error(err, "Error: failed to create compute kernel\n");
	}

	size_t state_length = 512 * 1024 * 1024;

	mem_state_pinned = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR, state_length, NULL, &err);
	if (err != CL_SUCCESS)
	{
		cl_print_error(err,  "Error: failed to allocate device memory (state_pinned, size: %lu)\n", state_length);
	}

	mem_state = clCreateBuffer(context, CL_MEM_READ_WRITE, state_length, NULL, &err);
	if (err != CL_SUCCESS)
	{
		cl_print_error(err,  "Error: failed to allocate device memory (state, size: %lu)\n", state_length);
	}

	err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &mem_state);
	if (err != CL_SUCCESS)
	{
		cl_print_error(err, "Error: failed to bind kernel argument \"mem_state\" [#0]\n");
	}

	uint8_t *data = (uint8_t *) clEnqueueMapBuffer(queue, mem_state_pinned, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, state_length, 0, NULL, NULL, &err); 
	if (err != CL_SUCCESS) {
		cl_print_error(err, "Error: failed map buffer (state_pinned)\n");
	}

	for (size_t i =0; i < state_length; i++)
	{
		data[i] = i % 1024;
	}

	err = clEnqueueWriteBuffer(queue, mem_state, CL_FALSE, 0, state_length, data, 0, NULL, NULL);
	if (err != CL_SUCCESS) {
		cl_print_error(err, "Error: failed write data to buffer (state)\n");
	}

	size_t global = state_length / 16;
	size_t local = 256;

	err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global, &local, 0, NULL, NULL);
	if (err != CL_SUCCESS) {
		cl_print_error(err, "Error: failed to execute kernel\n");
	}

	err = clEnqueueReadBuffer(queue, mem_state, CL_TRUE, 0, state_length, data, 0, NULL, NULL);
	if (err != CL_SUCCESS) {
		cl_print_error(err, "Error: failed read data from buffer (state)\n");
	}

	cl_release_all();

	printf("Done\n");
	system("pause");
	return 0;
}

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

	system("pause");
	exit(EXIT_FAILURE);
}

void cl_release_kernel()
{
	if (mem_state != nullptr)
	{
		while (clReleaseMemObject(mem_state));
		mem_state = nullptr;
	}
	
	if (mem_state_pinned != nullptr)
	{
		while (clReleaseMemObject(mem_state_pinned));
		mem_state_pinned = nullptr;
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