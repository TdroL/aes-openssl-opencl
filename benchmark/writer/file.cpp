#include "stdafx.h"
#include "file.h"
#include <iomanip>

#include <iostream>

namespace Writer
{

using namespace std;

File::File(string &target)
{
	file.open(target, fstream::out | fstream::trunc);

	desc = "file (" + target + ")";
}

bool File::ready()
{
	return file.is_open();
}

void File::write(int64_t duration, unsigned int step, unsigned int total)
{
	cout << ".";

	step++;
	if (step % 50 == 0 || step == total)
	{
		cout << "  " << (step * 100 / total) << "%" << endl;
	}
	else if (step % 10 == 0)
	{
		cout << " ";
	}

	file << duration << endl;
}

File::~File()
{
	file.close();
}

}