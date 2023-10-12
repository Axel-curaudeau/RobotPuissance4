
#if !defined(FFDYNARRAY_HPP)
#define FFDYNARRAY_HPP

#include <vector>
#include <initializer_list>
#include <iostream>

#include "ffmath.hpp"


namespace ff { template<typename T, typename U> struct mapdynarray; }

namespace ff
{

	template<typename T> struct dynarray
	{
	private:
		std::vector<T> storage;

	public:
		// standard constructors/destructors:
		dynarray();
		dynarray(const dynarray& _copy);
		dynarray(dynarray&& _move) noexcept;
		dynarray& operator=(const dynarray& _copy);
		dynarray& operator=(dynarray&& _move) noexcept;

		// custom constructors:
		dynarray(std::initializer_list<T> _list);

		// basic usage:
		void pushback(const T& _value);
		void pushback(const dynarray<T>& _dynarray);
		T popback();
		void reserve(uint _alloc);
		void resize(uint _size);
		// insert(2, X)   [0] [1] [2] [3] [4] [5] [6]
		//       before:  (a   b   c   d   e   f)
		//        after:  (a   b   X   c   d   e   f)
		void insert(uint _index, const T& _value);
		// erase(2)   [0] [1] [2] [3] [4] [5]
		//   before:  (a   b   c   d   e   f)
		//    after:  (a   b   d   e   f)
		void erase(uint _index);
		void erase(uint _idxStart, uint _idxEnd);
		T pop(uint _index);
		T operator[](uint _index) const;
		T& operator[](uint _index);
		T back() const;
		T& back();
		uint size() const;
		void clear();
		T* data();
		const T* data() const;

		// advanced usage:
		bool operator==(const dynarray& _other);
		uint contains(const T& _elem);
		void fill(const T& _elem);
		void mirror();
		bool find(const T& _element) const;
		bool find(const T& _element, uint& _idx) const;
		bool findAtIndex(const dynarray<T>& _sequence, uint _idx) const;

		/// \brief Sorts (smallest first, biggest last)
		/// \return Map from old idx to new idx
		ff::mapdynarray<uint, uint> sort();

		void sort(ff::mapdynarray<uint, uint> _sortMap);
		ff::mapdynarray<T, uint> getIdxMap();
	};

	template<> struct dynarray<bool> : public dynarray<char> {}; // Specialization
}


#include "ffmapdynarray.hpp"

template<typename T> ff::dynarray<T>::dynarray() { storage = std::vector<T>(); }
template<typename T> ff::dynarray<T>::dynarray(const dynarray& _copy) { storage = _copy.storage; }
template<typename T> ff::dynarray<T>::dynarray(dynarray&& _move) noexcept { storage = _move.storage; }
template<typename T> ff::dynarray<T>& ff::dynarray<T>::operator=(const dynarray& _copy) { storage = _copy.storage; return *this; }
template<typename T> ff::dynarray<T>& ff::dynarray<T>::operator=(dynarray&& _move) noexcept { storage = _move.storage; return *this; }
template<typename T> ff::dynarray<T>::dynarray(std::initializer_list<T> _list) { storage = std::vector<T>(_list); }
template<typename T> void ff::dynarray<T>::pushback(const T& _value) { storage.push_back(_value); }
template<typename T> void ff::dynarray<T>::pushback(const ff::dynarray<T>& _dynarray) { for (uint i = 0; i < _dynarray.size(); i += 1) { pushback(_dynarray[i]); } }
template<typename T> T ff::dynarray<T>::popback()
{
	if (size() == 0) { std::cout << "Dynarray - popback() out of range"; }
	
	T cpy = back();
	storage.pop_back();
	return cpy;
}
template<typename T> void ff::dynarray<T>::reserve(uint _alloc)
{
	storage.reserve(_alloc);
}
template<typename T> void ff::dynarray<T>::resize(uint _size)
{
	storage.resize(_size);
}
template<typename T> void ff::dynarray<T>::insert(uint _index, const T& _value)
{
	storage.insert(storage.begin() + _index, _value);
}
template<typename T> void ff::dynarray<T>::erase(uint _index)
{
	if (_index >= size()) { std::cout << "Out of range erase (" << _index << "/" << size() << ")"; }

	storage.erase(storage.begin() + _index);
}
template<typename T> void ff::dynarray<T>::erase(uint _idxStart, uint _idxEnd)
{
	if (_idxEnd > size()) { std::cout << "End range erase is too far"; }
	if (_idxStart > _idxEnd) { std::cout << "Wrong range erase (end idx is smaller than start idx)"; }

	for (uint i = _idxStart; i < _idxEnd; i += 1) { erase(_idxStart); }
}
template<typename T> T ff::dynarray<T>::pop(uint _index)
{
	if (_index >= size()) { std::cout << "Dynarray - pop() out of range"; }

	T cpy = (*this)[_index];
	erase(_index);
	return cpy;
}
template<typename T> T ff::dynarray<T>::operator[](uint _idx) const
{
	if (_idx >= size()) { std::cout << "Out of range const access (" << _idx << "/" << size() << ")"; }
	return storage[_idx];
}
template<typename T> T& ff::dynarray<T>::operator[](uint _index)
{
	if (_index >= size())
	{
		std::cout << "Out of range ref access (" << _index << "/" << size() << ")\n";
	}
	return storage[_index];
}
template<typename T> T ff::dynarray<T>::back() const
{
	if (size() == 0) { std::cout << "Dynarray - back() out of range\n"; }
	return storage[size() - 1];
}
template<typename T> T& ff::dynarray<T>::back()
{
	if (storage.size() <= 0) { std::cout << "Dynarray - back()& out of range\n"; }
	return storage[size() - 1];
}
template<typename T> uint ff::dynarray<T>::size() const { return (uint)storage.size(); }
template<typename T> void ff::dynarray<T>::clear() { storage.clear(); }
template<typename T> T* ff::dynarray<T>::data() { return storage.data(); }
template<typename T> const T* ff::dynarray<T>::data() const { return storage.data(); }

template<typename T> bool ff::dynarray<T>::operator==(const dynarray& _other)
{
	if (size() != _other.size()) { return false; }
	for (uint i = 0; i < size(); i += 1) { if ((*this)[i] != _other[i]) { return false; } }
	return true;
}

template<typename T> uint ff::dynarray<T>::contains(const T& _elem)
{
	uint count = 0;
	for (uint i = 0; i < size(); i += 1)
	{
		if ((*this)[i] == _elem) { count += 1; }
	}

	return count;
}
template<typename T> void ff::dynarray<T>::fill(const T& _elem)
{
	for (uint i = 0; i < size(); i += 1)
	{
		(*this)[i] = _elem;
	}
}
template<typename T> void ff::dynarray<T>::mirror()
{
	for (uint i = 0; i < size() / 2; i += 1)
	{
		T cpy = (*this)[i];
		(*this)[i] = (*this)[size() - 1 - i];
		(*this)[size() - 1 - i] = cpy;
	}
}
template<typename T> bool ff::dynarray<T>::find(const T& _element) const
{
	for (uint i = 0; i < size(); i += 1) { if ((*this)[i] == _element) { return true; } }
	return false;
}
template<typename T> bool ff::dynarray<T>::find(const T& _element, uint& _idx) const
{
	_idx = (uint)-1;
	for (uint i = 0; i < size(); i += 1) { if ((*this)[i] == _element) { _idx = i; return true; } }
	return false;
}
template<typename T> bool ff::dynarray<T>::findAtIndex(const ff::dynarray<T>& _sequence, uint _idx) const
{
	if (_sequence.size() + _idx > size()) { return false; }

	bool match = true;
	for (uint i = 0; i < _sequence.size(); i += 1)
	{
		if ((*this)[_idx + i] != _sequence[i]) { match = false; break; }
	}
	if (match) { return true; }
	return false;
}

template<typename T> ff::mapdynarray<uint, uint> ff::dynarray<T>::sort()
{
	ff::mapdynarray<uint, uint> result;
	for (uint i = 0; i < size(); i += 1) { result.pushback(i, i); }


	for (uint i = 0; i < size() - 1; i += 1)
	{
		uint minIdx = i;
		for (uint j = i + 1; j < size(); j += 1) { if ((*this)[j] < (*this)[minIdx]) { minIdx = j; } }
		if (minIdx == i) { continue; }

		T cpy = (*this)[i];
		(*this)[i] = (*this)[minIdx];
		(*this)[minIdx] = cpy;

		uint idxCpy = result[i];
		result[i] = result[minIdx];
		result[minIdx] = idxCpy;
	}

	return result;
}
template<typename T> void ff::dynarray<T>::sort(ff::mapdynarray<uint, uint> _sortMap)
{
	if (_sortMap.size() != size()) { return; }
	ff::dynarray<T> copy = *this;
	for (uint i = 0; i < size(); i += 1) { (*this)[_sortMap[i]] = copy[i]; }
}
template<typename T> ff::mapdynarray<T, uint> ff::dynarray<T>::getIdxMap()
{
	ff::mapdynarray<T, uint> result;
	for (uint i = 0; i < size(); i += 1) { result.pushback((*this)[i], i); }
	return result;
}

#endif
