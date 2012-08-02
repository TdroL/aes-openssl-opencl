#ifndef BENCH_BASE_H
#define BENCH_BASE_H

namespace Bench
{

class Base
{
public:
	typedef uint8_t* data_type;

	virtual bool init() = 0;
	virtual int64_t run(Base::data_type data, size_t size) = 0;
	virtual bool release() = 0;

	virtual ~Base();
};

}

#endif
