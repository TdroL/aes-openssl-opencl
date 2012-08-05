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
	length += length % 128;

	Bench::Container *sample = new Bench::Container(length);
	memset(sample->data, 1, length * sizeof(*(sample->data)));

	return unique_ptr<Bench::Container>(sample);
}

Dummy::~Dummy()
{
}

}