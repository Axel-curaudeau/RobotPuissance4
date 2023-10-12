
#pragma once

#include "ffsetup.hpp"
#include <iostream>

namespace ff
{
	struct rawmem
	{
		byte* address = nullptr;
		uint size = 0;

		/// \brief Nullptr default constructor
		rawmem();
		/// \brief Full memory area constructor
		rawmem(byte* _mem, uint _size);

		template<typename T> T& getAtByte(uint _byteOffset);
		rawmem getSubMem(uint _startOffset);
		rawmem getSubMem(uint _startOffset, uint _size);
		template<uint SPLITS> rawmem split(uint _idx);
		void zero();
		uint copyTo(rawmem _destination) const;
		template<typename T> uint write(T _first);
		template<typename T, typename U> uint write(T _first, U _second);
		template<typename T, typename U, typename V> uint write(T _first, U _second, V _third);


		template<typename T> T read();

		template<typename T, typename U> struct ret2 { T first; U second; };
		template<typename T, typename U> ret2<T, U> read();
		//ff::string getStringHex();
		void printString() const;
	};
}




ff::rawmem::rawmem() { address = nullptr; size = 0; }
ff::rawmem::rawmem(byte* _mem, uint _size) { address = _mem; size = _size; }
template<typename T> T& ff::rawmem::getAtByte(uint _byteOffset)
{
	return *((T*)&address[_byteOffset]);
}
ff::rawmem ff::rawmem::getSubMem(uint _startOffset)
{
	if (_startOffset > size) { std::cout << "[!] FFRAWMEM Submem requested is invalid\n"; return ff::rawmem(); }
	return ff::rawmem(address + _startOffset, size - _startOffset);
}
ff::rawmem ff::rawmem::getSubMem(uint _startOffset, uint _size)
{
	return ff::rawmem(address + _startOffset, _size);
}
template<uint SPLITS> ff::rawmem ff::rawmem::split(uint _idx)
{
	uint newSize = size / SPLITS;
	return ff::rawmem(address + (newSize * _idx), newSize);
}
void ff::rawmem::zero() { std::memset(address, 0, size); }
uint ff::rawmem::copyTo(rawmem _destination) const
{
	if (_destination.size < size) { return 0; }
	std::memcpy(_destination.address, address, size);
	return size;
}
template<typename T> uint ff::rawmem::write(T _first)
{
	if (sizeof(T) > size) { return 0; }

	getAtByte<T>(0) = _first;
	return sizeof(T);
}
template<typename T, typename U> uint ff::rawmem::write(T _first, U _second)
{
	if (sizeof(T) + sizeof(U) > size) { return 0; }

	getAtByte<T>(0) = _first;
	getAtByte<U>(sizeof(T)) = _second;
	return sizeof(T) + sizeof(U);
}
template<typename T, typename U, typename V> uint ff::rawmem::write(T _first, U _second, V _third)
{
	if (sizeof(T) + sizeof(U) + sizeof(V) > size) { return 0; }

	getAtByte<T>(0) = _first;
	getAtByte<U>(sizeof(T)) = _second;
	getAtByte<V>(sizeof(T) + sizeof(U)) = _third;
	return sizeof(T) + sizeof(U) + sizeof(V);
}
template<typename T> T ff::rawmem::read() { return getAtByte<T>(0); }
template<typename T, typename U> ff::rawmem::ret2<T, U> ff::rawmem::read() { ret2<T, U> result; result.first = getAtByte<T>(0); result.second = getAtByte<U>(sizeof(T)); return result; }
/*ff::string ff::rawmem::getStringHex()
{
	ff::string result = "0x ";
	for (uint i = 0; i < size; i += 1)
	{

		byte num = (address[i] & 0xf0) >> 4;
		if (num < 10) { result.pushback(ff::string((char)('0' + num))); }
		else { result.pushback(ff::string((char)('a' + (num - 10)))); }

		num = (address[i] & 0x0f);
		if (num < 10) { result.pushback(ff::string((char)('0' + num))); }
		else { result.pushback(ff::string((char)('a' + (num - 10)))); }


		if (i + 1 < size) { result.pushback(' '); }
	}

	std::cout << "\n";
	return result;
}*/
void ff::rawmem::printString() const
{
	for (uint i = 0; i < size; i += 1)
	{
		byte num = (address[i] & 0xf0) >> 4;
		if (num < 10) { std::cout << (char)('0' + num); }
		else { std::cout << (char)('a' + (num - 10)); }


		num = (address[i] & 0x0f);
		if (num < 10) { std::cout << (char)('0' + num); }
		else { std::cout << (char)('a' + (num - 10)); }

		std::cout << "(" << +address[i];
		char ascii = ' ';
		if (address[i] >= 32) { ascii = (char)address[i]; std::cout << "," << ascii; }
		std::cout << ") ";


		if (i + 1 < size) { std::cout << " "; }
	}

	std::cout << "\n";
}

