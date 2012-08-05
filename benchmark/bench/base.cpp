#include "stdafx.h"
#include "base.h"

namespace Bench
{

Container::Container(size_t length_)
	: length(length_)
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

Base::~Base()
{
}

}