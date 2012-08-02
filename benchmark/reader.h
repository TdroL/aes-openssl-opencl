#ifndef READER_H
#define READER_H

#include <string>
#include "bench/base.h"

class Reader
{
public:
	std::string desc;

	static std::unique_ptr<Reader> factory(std::string &target);

	virtual bool ready() = 0;
	virtual std::unique_ptr<Bench::Base::data_type> read(size_t length) = 0;

	virtual ~Reader();
};

#endif