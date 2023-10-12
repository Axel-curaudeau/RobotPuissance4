
#pragma once

#include "ffsetup.hpp"
#include <ostream>

namespace ff
{
	template<typename T> struct id
	{
		uint value;

		id();
		id(uint _id);

		bool operator==(const id<T>& _r) const;
		bool operator!=(const id<T>& _r) const;
		bool operator<(const id<T>& _r) const;
		bool isValid();
	};

	template<typename T> std::ostream& operator<<(std::ostream& _l, const ff::id<T>& _r);


	template<typename T> struct compidmanager
	{
		uint nextId = 0;

		id<T> addNew();
	};
}








template<typename T> ff::id<T>::id() { value = (uint)-1; }
template<typename T> ff::id<T>::id(uint _id) { value = _id; }
template<typename T> bool ff::id<T>::operator==(const id<T>& _r) const { return value == _r.value; }
template<typename T> bool ff::id<T>::operator!=(const id<T>& _r) const { return !(*this == _r); }
template<typename T> bool ff::id<T>::operator<(const id<T>& _r) const { return value < _r.value; }
template<typename T> bool ff::id<T>::isValid() { return value != (uint)-1; }
template<typename T> std::ostream& ff::operator<<(std::ostream& _l, const ff::id<T>& _r) { _l << _r.value; return _l; }


template<typename T> ff::id<T> ff::compidmanager<T>::addNew() { id<T> result = id<T>(nextId); nextId += 1; return result; }


