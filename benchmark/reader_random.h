#ifndef READER_RANDOM_H
#define READER_RANDOM_H

#include "reader.h"

class Reader_Random : public Reader
{
public:
	Reader_Random();

	bool ready();
	std::unique_ptr<Bench::Base::data_type> read(size_t length);

	~Reader_Random();
};

#endif