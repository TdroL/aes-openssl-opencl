#include "stdafx.h"
#include "benchmark.h"

using namespace std;

void Benchmark::registerBench(string name, unique_ptr<Bench::Base> &&bench)
{
	benchs.insert(name, bench.release());
}

bool Benchmark::run(Reader::Base &reader, Writer::Base &writer, string benchName, unsigned int loops, size_t sampleSize)
{
	auto pairFound = benchs.find(benchName);

	if (pairFound == benchs.end())
	{
		cerr << "Wrong benchmark name.\nAvaible benchmarks (" << benchs.size() << "):";
		for (auto it = benchs.begin(); it != benchs.end(); it++)
		{
			cerr << " " << it->first;
		}
		cerr << endl;

		return false;
	}

	Bench::Base *bench = pairFound->second;
	assert(bench != nullptr);

	cout << "Input file: " 
	     << reader.desc << endl;
	cout << "Writing results to: " 
	     << writer.desc << endl;
	cout << "Benchmark: " 
	     << benchName << " " << bench << endl;
	cout << "Loops count: " 
	     << loops << endl;
	cout << "Samples size: " 
	     << sampleSize << endl;

	if ( ! bench->init())
	{
		cerr << "Bench(" <<  pairFound->first << ")#init failed" << endl;
		return false;
	}

	for (size_t i = 0; i < loops; i++)
	{
		unique_ptr<Bench::Base::data_type> data(reader.read(sampleSize));
		assert(data != nullptr);

		int64_t dt = bench->run(*data, sampleSize);

		writer.write(dt);

		if ( dt < 0)
		{
			cerr << "Bench(" <<  pairFound->first << ")#run(data["<< sampleSize <<"]) failed" << endl;
			return false;
		}
	}

	if ( ! bench->release())
	{
		cerr << "Bench(" <<  pairFound->first << ")#release failed" << endl;
		return false;
	}

	return true;
}