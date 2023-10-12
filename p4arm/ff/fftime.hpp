
#pragma once

#include <Windows.h>

#include <chrono>
#include "ffsetup.hpp"

namespace ff
{
	struct timer
	{
		std::chrono::high_resolution_clock::time_point lastTime;

		timer();

		uint getMilli() const;
		uint getMicro() const;
		uint getNano() const;

		void restart();

		uint stopwatchMilli();
		uint stopwatchNano();

		/// \brief Check if amount of time has passed
		bool waitedForMilli(uint _ms) const;

		/// \brief Periodically check if amount of time has passed
		/// \details 
		bool tickEveryMilli(uint _ms);
	};


	timer sleepTimer = timer();
	void sleep(uint _milliseconds) { Sleep(_milliseconds); }
}


ff::timer::timer() { lastTime = std::chrono::high_resolution_clock::now(); }
uint ff::timer::getNano() const { return (uint)((std::chrono::duration<double, std::nano>)(std::chrono::high_resolution_clock::now() - lastTime)).count(); }
uint ff::timer::getMicro() const { return (uint)((std::chrono::duration<double, std::micro>)(std::chrono::high_resolution_clock::now() - lastTime)).count(); }
uint ff::timer::getMilli() const { return (uint)((std::chrono::duration<double, std::milli>)(std::chrono::high_resolution_clock::now() - lastTime)).count(); }
void ff::timer::restart() { lastTime = std::chrono::high_resolution_clock::now(); }
uint ff::timer::stopwatchMilli() { uint milli = getMilli(); restart(); return milli; }
uint ff::timer::stopwatchNano() { uint nano = getNano(); restart(); return nano; }
bool ff::timer::waitedForMilli(uint _ms) const { return getMilli() > _ms; }
bool ff::timer::tickEveryMilli(uint _ms) { if (getMilli() > _ms) { lastTime += std::chrono::milliseconds(_ms); return true; } else { return false; } }

