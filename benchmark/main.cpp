#include "stdafx.h"

#define OPENCL_UNSAFE_OPTIMIZATIONS

//#define MEMORY_LEAK_DETECTOR
#if defined(MEMORY_LEAK_DETECTOR) && defined(_DEBUG) && defined(WIN32)
	#define _CRTDBG_MAP_ALLOC 1
	#include <stdlib.h>
	#include <crtdbg.h>

    #define DBG_NEW new (_NORMAL_BLOCK, __FILE__, __LINE__)
    #define new DBG_NEW
#endif

#include "benchmark.h"
#include "boost/program_options.hpp"
#include "bench.h"
#include "reader.h"
#include "writer.h"
#include <sstream>

using namespace std;
typedef unsigned int uint;

namespace po = boost::program_options;

int main(int argc, char* argv[])
{
#if defined(MEMORY_LEAK_DETECTOR) && defined(_DEBUG) && defined(WIN32)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG | _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
#endif

	po::options_description desc("Options");
	po::positional_options_description p;
	po::variables_map vm;
	p.add("file", -1);

	desc.add_options()
		("help,h",                                      "help message")
		("file,f",         po::value<string>()
							->default_value(""),        "file to process")
		("output,o",       po::value<string>()
							->default_value(""),        "output file")
		("benchmark,b",    po::value<string>()
		                    ->default_value("aes-gpu"), "benchmark to run")
		("loops,l",        po::value<uint>()
		                    ->default_value(250),       "number of loops")
		("sample-size,s",  po::value<string>()
		                    ->default_value("1024"),    "sample size [suffix: none,K,M]")
		("key-length,k",   po::value<uint>()
		                     ->default_value(256),      "key length [128,192,256]")
		;

	Bench::Base::desc = &desc;
	Bench::Base::vm = &vm;
	Bench::Base::path = argv[0];

	Benchmark bm;
	bm.registerBench("aes-cpu",   Bench::factory<Bench::Aes::Cpu>());
	bm.registerBench("aes-gpu",   Bench::factory<Bench::Aes::Gpu>());
	bm.registerBench("aes-p-cpu", Bench::factory<Bench::Aes::P::Cpu>());
	bm.registerBench("aes-p-gpu", Bench::factory<Bench::Aes::P::Gpu>());

	po::store(po::command_line_parser(argc, argv).
			  options(desc).positional(p).run(), vm);
	po::notify(vm);

	if (vm.count("help"))
	{
		cout << desc << endl;
		return EXIT_SUCCESS;
	}

	string file = vm["file"].as<string>();
	string output = vm["output"].as<string>();
	string benchName = vm["benchmark"].as<string>();
	uint loops = vm["loops"].as<uint>();
	uint sampleSize = 0;
	string sampleSizeString = vm["sample-size"].as<string>();
	uint keyLength = vm["key-length"].as<uint>();

	if (sampleSizeString.size() > 0)
	{
		char suffix = sampleSizeString[sampleSizeString.length() - 1];

		istringstream iss(sampleSizeString);

		iss >> sampleSize;

		switch (suffix)
		{
		case 'M':
		case 'm':
			sampleSize *= 1024;
		case 'K':
		case 'k':
			sampleSize *= 1024;
		}
	}

	auto reader = Reader::factory(file);
	if ( ! reader->ready())
	{
		cerr << "Cannot open input file" << endl;
		return EXIT_FAILURE;
	}

	auto writer = Writer::factory(output);
	if ( ! writer->ready())
	{
		cerr << "Cannot open or create output file" << endl;
		return EXIT_FAILURE;
	}

	if (keyLength != 128 && keyLength != 192 && keyLength != 256)
	{
		cerr << "Invalid key length" << endl;
		return EXIT_FAILURE;
	}

	if ( ! bm.run(*reader, *writer, benchName, loops, sampleSize, keyLength))
	{
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

