#include "stdafx.h"
#include "cpu.h"
#include "boost/chrono.hpp"
#include "boost/thread.hpp"

namespace Bench
{
namespace Aes
{

namespace ch = boost::chrono;
typedef ch::high_resolution_clock hrc;

bool Cpu::init()
{
	return true;
}

int64_t Cpu::run(Bench::Base::data_type data, size_t size)
{
	auto start = hrc::now();

	boost::this_thread::sleep(boost::posix_time::milliseconds(10)); 

	auto end = hrc::now();
	return ch::duration_cast<ch::milliseconds>(end - start).count();
}

bool Cpu::release()
{
	return true;
}

Cpu::~Cpu()
{
}

}

std::unique_ptr<Aes::Cpu> factory()
{
	return std::unique_ptr<Aes::Cpu>(new Aes::Cpu);
}

}