#include "StdAfx.h"
#include "reader_random.h"

using namespace std;

Reader_Random::Reader_Random()
{
	desc = "none, random data";
}

bool Reader_Random::ready()
{
	return true;
}

unique_ptr<Bench::Base::data_type> Reader_Random::read(size_t length)
{
	unique_ptr<Bench::Base::data_type> data(new Bench::Base::data_type[length]);

	return data;
}

Reader_Random::~Reader_Random()
{
}
