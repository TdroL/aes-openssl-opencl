#include "stdafx.h"
#include "base.h"

namespace Bench
{

Container::Container(size_t length)
	: length(length)
{
	data = new unsigned char[length];
}
	
Container::~Container()
{
	if (data != nullptr)
	{
		delete[] data;
	}
}

const size_t Base::stateSize = 16;
boost::program_options::options_description *Base::desc = nullptr;
boost::program_options::variables_map *Base::vm = nullptr;
boost::filesystem::path Base::path;

void Base::add_options()
{
}

Base::~Base()
{
}

template<class T>
std::unique_ptr<T> factory()
{
	return std::unique_ptr<T>(new T);
}

}