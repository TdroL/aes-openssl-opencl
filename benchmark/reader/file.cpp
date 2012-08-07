#include "stdafx.h"
#include "file.h"

namespace Reader
{

using namespace std;

File::File(string target)
{
	file.open(target);

	desc = "file: " + target;
}

bool File::ready()
{
	return file.is_open();
}

unique_ptr<Bench::Container> File::read(size_t length)
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

File::~File()
{
	file.close();
}

}
