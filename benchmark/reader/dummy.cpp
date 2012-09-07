#include "stdafx.h"
#include "dummy.h"
#include "boost/random/mersenne_twister.hpp"
#include "boost/random/uniform_int_distribution.hpp"

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

static boost::random::mt19937 gen;
static boost::random::uniform_int_distribution<> dist(0, (1 << (8 * sizeof(Bench::Container::data_type))) - 1);

unique_ptr<Bench::Container> Dummy::read(size_t length)
{
	if (length % Bench::Base::stateSize != 0)
	{
		length += Bench::Base::stateSize - length % Bench::Base::stateSize;
	}

	Bench::Container *sample = new Bench::Container(length);
	assert(sample->data != nullptr);

	generate_n(sample->data, length, []() -> Bench::Container::data_type { return dist(gen); });

	return unique_ptr<Bench::Container>(sample);
}

bool Dummy::write(Bench::Container &sample)
{
	return true;
}

Dummy::~Dummy()
{
}

}