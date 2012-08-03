#include "stdafx.h"
#include "writer.h"

namespace Writer
{

using namespace std;

unique_ptr<Base> factory(string &target)
{
	if ( ! target.empty())
	{
		return unique_ptr<Base>(new File(target));
	}

	return unique_ptr<Base>(new Console);
}

}