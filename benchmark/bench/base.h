#ifndef BENCH_BASE_H
#define BENCH_BASE_H

#include "../bench/base.h"

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
	virtual bool init() = 0;
	virtual int64_t run(Bench::Container &sample) = 0;
	virtual bool release() = 0;

	virtual ~Base();
};

template<class T>
std::unique_ptr<T> factory();

}

#endif
