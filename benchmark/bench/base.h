#ifndef BENCH_BASE_H
#define BENCH_BASE_H

#include "../bench/base.h"
#include "boost/program_options.hpp"

namespace Bench
{

struct Container
{
	unsigned char *data;
	size_t length;

	Container(size_t length_);
	~Container();
};

class Base
{
public:
	static boost::program_options::options_description *desc;
	static boost::program_options::variables_map *vm;
	std::string errMsg;
	static const size_t stateSize;

	virtual bool init(size_t sample_length) = 0;
	virtual int64_t run(Bench::Container &sample) = 0;
	virtual bool release() = 0;

	virtual ~Base();
};

template<class T>
std::unique_ptr<T> factory();

}

#endif
