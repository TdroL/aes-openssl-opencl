#include "stdafx.h"
#include "benchmark.h"

using namespace std;

void Benchmark::registerBench(std::string name, Bench *bench)
{
	assert(bench != nullptr);
	benchs.insert(name, bench);
}