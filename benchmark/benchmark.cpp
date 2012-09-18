#include "stdafx.h"
#include "benchmark.h"

using namespace std;

void Benchmark::registerBench(string name, unique_ptr<Bench::Base> &&bench)
{
	benchs.insert(name, bench.release());
}

bool Benchmark::run(Reader::Base &reader, Writer::Base &writer, string &benchName, unsigned int loops, size_t sampleSize, unsigned int keyLength)
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

	clog << "Input file: "
	     << reader.desc << endl;
	clog << "Writing results to: "
	     << writer.desc << endl;
	clog << "Benchmark: "
	     << benchName << endl;
	clog << "Loops count: "
	     << loops << endl;
	clog << "Sample size: "
	     << sampleSize << " bytes" << endl;

	unique_ptr<Bench::Container> sample(reader.read(sampleSize));
	assert(sample != nullptr);
	assert(sample->data != nullptr);
	assert(sample->length % Bench::Base::stateSize == 0);

	if ( ! bench->init(sample->length, static_cast<size_t>(keyLength)))
	{
		cerr << "Bench(" <<  pairFound->first << ")#init failed" << endl;
		if ( ! bench->errMsg.empty())
		{
			cerr << "Message: " << bench->errMsg << endl;
		}

		bench->release();
		return false;
	}

	bool status = true;
	for (size_t i = 0; i < loops; i++)
	{
		string dt = bench->run(*sample);

		if (dt.empty())
		{
			cerr << "Bench(" <<  pairFound->first << ")#run(sample["<< sampleSize <<"]) failed" << endl;
			if ( ! bench->errMsg.empty())
			{
				cerr << "Message: " << bench->errMsg << endl;
			}
			status = false;
			break;
		}

		writer.write(dt, i, loops);
	}

	if ( ! reader.write(*sample))
	{
		clog << "Warning: could not write to input file" << endl;
	}

	if ( ! bench->release())
	{
		cerr << "Bench(" <<  pairFound->first << ")#release failed" << endl;
		if ( ! bench->errMsg.empty())
		{
			cerr << "Message: " << bench->errMsg << endl;
		}
		return false;
	}

	return status;
}