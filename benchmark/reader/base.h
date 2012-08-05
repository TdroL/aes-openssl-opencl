#ifndef READER_BASE_H
#define READER_BASE_H

#include <string>
#include "../bench/base.h"

namespace Reader
{

class Base
{
public:
	std::string desc;

	virtual bool ready() = 0;
	virtual std::unique_ptr<Bench::Container> read(size_t length) = 0;

	virtual ~Base();
};

}

#endif