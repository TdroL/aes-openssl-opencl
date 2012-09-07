#ifndef BENCH_BASE_H
#define BENCH_BASE_H

#include "../bench/base.h"
#include "boost/program_options.hpp"
#include "boost/filesystem.hpp"
#include "boost/lexical_cast.hpp"

#ifndef KEYLENGTH
	#define KEYLENGTH(keybits) ((keybits)/8)
#endif
#ifndef RKLENGTH
	#define RKLENGTH(keybits)  ((keybits)/8+28)
#endif
#ifndef NROUNDS
	#define NROUNDS(keybits)   ((keybits)/32+6)
#endif

namespace Bench
{

struct Container
{
	typedef unsigned char data_type;
	Container::data_type *data;
	size_t length;

	Container(size_t length_);
	~Container();
};

class Base
{
public:
	static boost::program_options::options_description *desc;
	static boost::program_options::variables_map *vm;
	static boost::filesystem::path path;
	std::string errMsg;
	static const size_t stateSize;

	static void add_options();

	virtual bool init(size_t sampleLength, size_t keyLength) = 0;
	virtual std::string run(Bench::Container &sample) = 0;
	virtual bool release() = 0;

	virtual ~Base();
};

template<class T>
std::unique_ptr<T> factory();

}

#endif
