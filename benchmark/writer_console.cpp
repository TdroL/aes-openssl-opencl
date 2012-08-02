#include "StdAfx.h"
#include "writer_console.h"
#include <iostream>

using namespace std;

Writer_Console::Writer_Console()
{
	desc = "console";
}

bool Writer_Console::ready()
{
	return true;
}

void Writer_Console::write(int64_t duration)
{
	cout << duration << endl;
}

Writer_Console::~Writer_Console()
{
}
