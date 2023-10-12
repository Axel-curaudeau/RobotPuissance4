
#if !defined(FFMATH_HPP)
#define FFMATH_HPP

#include "ffsetup.hpp"

namespace ff
{
	// maximum:
	template<typename T> T maxOf(T _first);
	template<typename T, typename... Tinf> T maxOf(T _first, Tinf... _rest);

	// minimum:
	template<typename T> T minOf(T _first);
	template<typename T, typename... Tinf> T minOf(T _first, Tinf... _rest);

	// absolute:
	template<typename T> T abs(T _num);

	// nextPow2(a) = b
	// a: | 0 | 1 | 2 | 3 | 4 | 5 |
	// b: | 1 | 1 | 2 | 4 | 4 | 8 |
	uint64 nextPow2(uint64 _num);








	template<typename T> T maxOf(T _first) { return _first; }
	template<typename T, typename... Tinf> T maxOf(T _first, Tinf... _rest)
	{
		if (_first >= maxOf(_rest...)) { return _first; }
		return maxOf(_rest...);
	}

	template<typename T> T minOf(T _first) { return _first; }
	template<typename T, typename... Tinf> T minOf(T _first, Tinf... _rest)
	{
		if (_first <= minOf(_rest...)) { return _first; }
		return minOf(_rest...);
	}

	template<typename T> T abs(T _num) { if (_num < 0) { return -_num; } else { return _num; } }

	
	uint64 nextPow2(uint64 _num)
	{
		// Implementation info: https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
		if (_num == 0) { return 1; }

		_num -= 1;
		for (uint i = 1; i < 8 * sizeof(_num); i *= 2)
		{
			_num |= (_num >> i);
		}
		_num += 1;
		return _num;
	}

	uint64 pow2(uint64 _power)
	{
		return (uint64)1 << _power;
	}
	
}

#endif
