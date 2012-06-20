#include "stdafx.h"
#include "benchmark.h"

using namespace std;

Benchmark::Timer::Timer()
{
#ifdef _WIN32
	bool has_qpf = QueryPerformanceFrequency(&freq);
	assert(has_qpf);
#endif
}

void Benchmark::Timer::start()
{
#ifdef _WIN32
	QueryPerformanceCounter(&t1);
#else
	gettimeofday(&t1, NULL);
#endif
}

void Benchmark::Timer::stop()
{
#ifdef _WIN32
	QueryPerformanceCounter(&t2);

	elapsed_time = (t2.QuadPart - t1.QuadPart) * 1000.0 / freq.QuadPart;
#else
	gettimeofday(&t2, NULL);

	elapsed_time = ((t2.tv_sec - t1.tv_sec) * 1000.0) + ((t2.tv_usec - t1.tv_usec) / 1000.0);
#endif
}

void Benchmark::registerBench(std::string name, Bench *bench, boost::program_options::options_description &desc)
{
	assert(bench != nullptr);
	benchs.insert(name, bench);
}