#include "stdafx.h"
#include "console.h"
#include <iostream>

namespace Writer
{

using namespace std;

Console::Console()
{
	desc = "console";
}

bool Console::ready()
{
	return true;
}

void Console::write(string &duration, unsigned int step, unsigned int total)
{
	cout << duration << endl;
}

Console::~Console()
{
}

}