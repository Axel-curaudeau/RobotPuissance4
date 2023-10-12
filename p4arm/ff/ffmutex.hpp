
#pragma once

#include <atomic>
#include <iostream>

namespace ff
{
	struct mutex
	{
		std::atomic<bool> isLocked;

		mutex();

		mutex& operator=(const mutex& _mtx);

		void lock();
		void unlock();
	};
}

ff::mutex::mutex()
{
	isLocked.store(false);
}
ff::mutex& ff::mutex::operator=(const mutex& _mtx)
{
	isLocked.store(_mtx.isLocked.load());
	return *this;
}
void ff::mutex::lock()
{
	while (isLocked.exchange(true)) {}
}
void ff::mutex::unlock()
{
	isLocked.store(false);
}