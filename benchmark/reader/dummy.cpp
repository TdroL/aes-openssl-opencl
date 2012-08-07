#include "stdafx.h"
#include "dummy.h"

namespace Reader
{

using namespace std;

Dummy::Dummy()
{
	desc = "none (random data)";
}

bool Dummy::ready()
{
	return true;
}

unique_ptr<Bench::Container> Dummy::read(size_t length)
{
	if (length % Bench::Base::stateSize != 0)
	{
		length += Bench::Base::stateSize - length % Bench::Base::stateSize;
	}

	Bench::Container *sample = new Bench::Container(length);
	assert(sample->data != nullptr);
	memset(sample->data, 1, length * sizeof(*(sample->data)));

	return unique_ptr<Bench::Container>(sample);
}

Dummy::~Dummy()
{
}

}