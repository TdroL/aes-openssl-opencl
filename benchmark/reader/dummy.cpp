#include "stdafx.h"
#include "dummy.h"

namespace Reader
{

using namespace std;

Dummy::Dummy()
{
	desc = "none, random data";
}

bool Dummy::ready()
{
	return true;
}

unique_ptr<Bench::Base::data_type> Dummy::read(size_t length)
{
	unique_ptr<Bench::Base::data_type> data(new Bench::Base::data_type[length]);

	return data;
}

Dummy::~Dummy()
{
}

}