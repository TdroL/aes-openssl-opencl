#ifndef WRITER_H
#define WRITER_H

#include "boost/chrono.hpp"
#include <string>

class Writer
{
public:
	std::string desc;

	static std::unique_ptr<Writer> factory(std::string &target);

	virtual bool ready() = 0;
	virtual void write(int64_t duration) = 0;

	virtual ~Writer();
};

#endif