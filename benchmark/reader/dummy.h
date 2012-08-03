#ifndef READER_DUMMY_H
#define READER_DUMMY_H

#include "base.h"

namespace Reader
{

class Dummy : public Reader::Base
{
public:
	Dummy();

	bool ready();
	std::unique_ptr<Bench::Base::data_type> read(size_t length);

	~Dummy();
};

}

#endif