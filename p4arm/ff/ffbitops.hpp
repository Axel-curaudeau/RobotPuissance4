
#pragma once

#include "ffsetup.hpp"

namespace ff
{
	namespace bitops
	{
		uint countBits(uint _value);
		uint countBits(uint64 _value);
	}
}


uint ff::bitops::countBits(uint _value)
{
	uint result = 0;
	while (_value > 0)
	{
		if (_value % 2 == 1) { result += 1; }
		_value = _value >> 1;
	}
	return result;
}
uint ff::bitops::countBits(uint64 _value)
{
	uint result = 0;
	while (_value > 0)
	{
		if (_value % 2 == 1) { result += 1; }
		_value = _value >> 1;
	}
	return result;
}
