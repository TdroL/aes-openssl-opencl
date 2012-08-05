#include "stdafx.h"
#include "gpu.h"
#include "boost/chrono.hpp"
#include "boost/thread.hpp"

namespace Bench
{
namespace Aes
{

namespace ch = boost::chrono;
typedef ch::high_resolution_clock hrc;

bool Gpu::init()
{
	return true;
}

int64_t Gpu::run(Bench::Container &sample)
{
	auto start = hrc::now();

	boost::this_thread::sleep(boost::posix_time::milliseconds(10)); 

	auto end = hrc::now();
	return ch::duration_cast<ch::milliseconds>(end - start).count();
}

bool Gpu::release()
{
	return true;
}

Gpu::~Gpu()
{
}

}

std::unique_ptr<Aes::Gpu> factory()
{
	return std::unique_ptr<Aes::Gpu>(new Aes::Gpu);
}

}