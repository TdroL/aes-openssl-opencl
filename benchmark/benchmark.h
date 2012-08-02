#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <string>
#include "bench/base.h"
#include "boost/ptr_container/ptr_map.hpp"
#include "writer.h"
#include "reader.h"

class Benchmark
{
public:
	boost::ptr_map<std::string, Bench::Base> benchs;

	bool run(Reader &reader, Writer &writer, std::string benchName, unsigned int loops, size_t sampleSize);
	void registerBench(std::string name, std::unique_ptr<Bench::Base> &&bench);
};

#endif