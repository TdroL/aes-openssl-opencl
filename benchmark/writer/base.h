#ifndef WRITER_BASE_H
#define WRITER_BASE_H

#include "boost/chrono.hpp"
#include <string>

namespace Writer
{

class Base
{
public:
	std::string desc;

	virtual bool ready() = 0;
	virtual void write(int64_t duration, unsigned int step, unsigned int total) = 0;

	virtual ~Base();
};

}

#endif