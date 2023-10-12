
#pragma once

#include "ffsetup.hpp"
#include "ffstring.hpp"

namespace ff
{
	template<typename T> struct interval
	{
		T start = -1;
		T end = -1;

		interval() {}
		interval(T _start, T _end) { start = _start; end = _end; }

		T size() { return end - start; }

		bool contains(T _num) { return _num >= start && _num < end; }

		bool operator==(interval _other) { return start == _other.start && end == _other.end; }
		bool overlaps(interval _other) { return start < _other.end && _other.start < end; }
		bool contains(interval _other) { return _other.start >= start && _other.end <= end; }
		bool isContainedBy(interval _other) { return _other.contains(*this); }
		bool hasStartCutBy(interval _other) { return _other.start <= start && contains(_other.end); }
		bool hasEndCutBy(interval _other) { return _other.end >= end && contains(_other.start); }

		ff::string getString() { return end == start ? "[-, -]" : ((ff::string)"[" + (ff::string)start + ", " + (ff::string)(end) + "["); }
		//void exclude(interval _region) {  }

		
		// lower the end if smaller than current end
		interval<T>& clampEnd(T _end) { if (_end < end) { end = _end; } return *this; }

		interval<T>& shrinkEndToFit(T _includedValue);

		// increase the start if bigger than current start
		interval<T>& shrinkStartToFit(T _start) { if (_start > start) { start = _start; } return *this; }

		T getMaxValue() { return end - 1; }
		T getMinValue() { return start; }
	};
}

ff::interval<uint8>& ff::interval<uint8>::shrinkEndToFit(uint8 _includedValue) { if (_includedValue < end) { end = _includedValue + 1; } return *this; }
ff::interval<uint16>& ff::interval<uint16>::shrinkEndToFit(uint16 _includedValue) { if (_includedValue < end) { end = _includedValue + 1; } return *this; }
ff::interval<uint32>& ff::interval<uint32>::shrinkEndToFit(uint32 _includedValue) { if (_includedValue < end) { end = _includedValue + 1; } return *this; }
ff::interval<uint64>& ff::interval<uint64>::shrinkEndToFit(uint64 _includedValue) { if (_includedValue < end) { end = _includedValue + 1; } return *this; }
ff::interval<int8>& ff::interval<int8>::shrinkEndToFit(int8 _includedValue) { if (_includedValue < end) { end = _includedValue + 1; } return *this; }
ff::interval<int16>& ff::interval<int16>::shrinkEndToFit(int16 _includedValue) { if (_includedValue < end) { end = _includedValue + 1; } return *this; }
ff::interval<int32>& ff::interval<int32>::shrinkEndToFit(int32 _includedValue) { if (_includedValue < end) { end = _includedValue + 1; } return *this; }
ff::interval<int64>& ff::interval<int64>::shrinkEndToFit(int64 _includedValue) { if (_includedValue < end) { end = _includedValue + 1; } return *this; }
template<typename T> ff::interval<T>& ff::interval<T>::shrinkEndToFit(T _includedValue) { if (_includedValue < end) { end = _includedValue; } return *this; }
