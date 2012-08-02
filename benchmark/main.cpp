#include "stdafx.h"
#include "benchmark.h"
#include "boost/program_options.hpp"
#include "bench/aes/cpu.h"
#include "bench/aes/gpu.h"
#include "writer.h"
#include "reader.h"

using namespace std;
typedef unsigned int uint;

namespace po = boost::program_options;

int main(int argc, char* argv[])
{
	
	Benchmark bm;
	bm.registerBench("aes-cpu", unique_ptr<Bench::Base>(new Bench::Aes::Cpu));
	bm.registerBench("aes-gpu", unique_ptr<Bench::Base>(new Bench::Aes::Gpu));

	po::options_description desc("Options");
	po::positional_options_description p;
	p.add("file", -1);

	desc.add_options()
		("help,h",                                      "help message")
		("file,f",         po::value<string>()
							->default_value(""),        "file to process")
		("output,o",       po::value<string>()
							->default_value(""),        "output file")
		("benchmark,b",    po::value<string>()
		                    ->default_value("aes-cpu"), "benchmark to run")
		("loops,l",        po::value<uint>()
		                    ->default_value(256),       "loops count")
		("samples-size,s", po::value<uint>()
		                    ->default_value(1024),      "samples size (used only if input file not provided)")
		;

	po::variables_map vm;
	po::store(po::command_line_parser(argc, argv).
			  options(desc).positional(p).run(), vm);
	po::notify(vm);   

	if (vm.count("help")) {
		cout << desc << endl;
		return EXIT_SUCCESS;
	}

	string file = vm["file"].as<string>();
	string output = vm["output"].as<string>();
	string benchName = vm["benchmark"].as<string>();
	uint loops = vm["loops"].as<uint>();
	size_t sampleSize = vm["samples-size"].as<uint>();

	unique_ptr<Reader> reader(Reader::factory(file));
	assert(reader != nullptr);
	if ( ! reader->ready())
	{
		cerr << "Cannot open input file" << endl;
	}

	unique_ptr<Writer> writer(Writer::factory(output));
	assert(writer != nullptr);
	if ( ! writer->ready())
	{
		cerr << "Cannot open or create output file" << endl;
	}

	if ( ! bm.run(*reader, *writer, benchName, loops, sampleSize))
	{
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

