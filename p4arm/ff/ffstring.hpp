
#pragma once


#include "ffdynarray.hpp"
#include <fstream>
#include <string>
#include "ffrawmem.hpp"

namespace ff
{
	struct string
	{
		dynarray<char> storage;

		string();
		string(const string& _str);
		string(const char _char);
		string(const char* _str);
		string(const std::string& _str);
		string(const int& _num);
		string(const uint& _num);
		string(const uint64& _num);

		string& operator =(const string& _r);
		bool operator ==(const string& _str) const;
		bool operator ==(const std::string& _str) const;
		bool operator ==(const char* _str) const;
		string operator +(const string& _str) const;
		string& operator +=(const string& _str);
		string operator +(const char* _string) const;

		bool operator<(const ff::string& _r) const;

		operator std::string() const;
		operator float();

		char operator[](uint _i) const;
		char& operator[](uint _i);
		void pushback(const string& _r);
		void erase(uint _idx);
		void erase(uint _idx, uint _length);
		uint size() const;
		const char* data() const;
		char* data();
		void mirror();

		bool find(const string& _str) const;
		bool findAtIndex(const string& _str, uint _idx) const;
		uint findIndex(const string& _str) const;
		string subString(const uint& _startIdx) const { string result; for (uint i = _startIdx; i < size(); i += 1) { result.pushback((*this)[i]); } return result; }
		string subString(const uint& _startIdx, const uint& _endIdx) const { string result; if (_endIdx > size()) { return result; } for (uint i = _startIdx; i < _endIdx; i += 1) { result.pushback((*this)[i]); } return result; }

		uint serialSave(ff::rawmem _target) const;
		uint getSerialSaveSize() const;
		uint serialLoad(ff::rawmem _target);

		bool saveToFile(string _fileName);
	};
}


ff::string::string()
{
	storage = { '\0' };

	//if (storage.size() >= 2 && storage[storage.size() - 2] == '\0' && storage[storage.size() - 1] == '\0') { std::cout << "[!] FOUND THIS CCC!!!\n"; std::cout << storage.data() << "\n"; }
}
ff::string::string(const string& _str)
{
	storage = _str.storage;

	//if (storage.size() >= 2 && storage[storage.size() - 2] == '\0' && storage[storage.size() - 1] == '\0') { std::cout << "[!] FOUND THIS DDDD!!!\n"; std::cout << storage.data() << "\n"; }
}
ff::string::string(const char _char)
{
	if (_char == '\0') { storage = { '\0' }; } else { storage = {_char, '\0'}; }

	//if (storage.size() >= 2 && storage[storage.size() - 2] == '\0' && storage[storage.size() - 1] == '\0') { std::cout << "[!] FOUND THIS ZZZZ!!!\n"; std::cout << storage.data() << "\n"; }
}
ff::string::string(const char* _str)
{
	storage.resize((uint)std::strlen(_str) + 1);
	for (uint i = 0; i < size(); i += 1) { storage[i] = _str[i]; }
	storage.back() = '\0';

	//if (storage.size() >= 2 && storage[storage.size() - 2] == '\0' && storage[storage.size() - 1] == '\0') { std::cout << "[!] FOUND THIS YYY!!!\n"; std::cout << storage.data() << "\n"; }
}
ff::string::string(const std::string& _str)
{
	storage.resize((uint)_str.size() + 1);
	for (uint i = 0; i < size(); i += 1) { storage[i] = _str[i]; }
	storage.back() = '\0';

	//if (storage.size() >= 2 && storage[storage.size() - 2] == '\0' && storage[storage.size() - 1] == '\0') { std::cout << "[!] FOUND THIS XXX!!!\n"; std::cout << storage.data() << "\n"; }
}
ff::string::string(const int& _num)
{
	if (_num == 0) { storage = { '0', '\0' }; return; }
	
	int n = _num;
	if (_num < 0) { n = -n; }
	while (n > 0) { storage.pushback('0' + n % 10); n /= 10; }
	if (_num < 0) { storage.pushback('-'); }
	storage.mirror();

	storage.pushback('\0');

	//if (storage.size() >= 2 && storage[storage.size() - 2] == '\0' && storage[storage.size() - 1] == '\0') { std::cout << "[!] FOUND THIS AAAAA!!!\n"; std::cout << storage.data() << "\n"; }
}
ff::string::string(const uint& _num)
{
	if (_num == 0) { storage.pushback('0'); storage.pushback('\0'); return; }

	uint n = _num;
	while (n > 0) { storage.pushback('0' + n % 10); n /= 10; }
	storage.mirror();

	storage.pushback('\0');

	//if (storage.size() >= 2 && storage[storage.size() - 2] == '\0' && storage[storage.size() - 1] == '\0') { std::cout << "[!] FOUND THIS BBBBBBB!!!\n"; std::cout << storage.data() << "\n"; }
}
ff::string::string(const uint64& _num)
{
	if (_num == 0) { storage.pushback('0'); storage.pushback('\0'); return; }

	uint64 n = _num;
	while (n > 0) { storage.pushback('0' + n % 10); n /= 10; }
	storage.mirror();

	storage.pushback('\0');

	//if (storage.size() >= 2 && storage[storage.size() - 2] == '\0' && storage[storage.size() - 1] == '\0') { std::cout << "[!] FOUND THIS BBBBBBB!!!\n"; std::cout << storage.data() << "\n"; }
}

ff::string& ff::string::operator =(const string& _r) { storage = _r.storage; return *this; }
bool ff::string::operator ==(const string& _str) const
{
	if (size() != _str.size()) { return false; }
	bool match = true;
	for (uint i = 0; i < size(); i += 1) { if ((*this)[i] != _str[i]) { match = false; break; } }
	return match;
}
bool ff::string::operator ==(const std::string& _str) const
{
	if (size() != _str.size()) { return false; }
	bool match = true;
	for (uint i = 0; i < size(); i += 1) { if ((*this)[i] != _str[i]) { match = false; break; } }
	return match;
}
bool ff::string::operator ==(const char* _str) const
{
	bool match = true;
	for (uint i = 0; i < size(); i += 1) { if (_str[i] == '\0') { match = false; break; } if ((*this)[i] != _str[i]) { match = false; break; } }
	return match;
}
ff::string ff::string::operator +(const string& _str) const
{
	string cpy = *this;
	uint oldSize = size();
	cpy.storage.resize(size() + _str.size() + 1);
	for (uint i = 0; i < _str.size(); i += 1) { cpy.storage[oldSize + i] = _str[i]; }
	cpy.storage.back() = '\0';

	if (cpy.storage.size() >= 2 && cpy.storage[cpy.storage.size() - 2] == '\0' && cpy.storage[cpy.storage.size() - 1] == '\0') { std::cout << "[!] COPYYY!!!\n"; std::cout << cpy.storage.data() << "\n"; }

	return cpy;
}
ff::string& ff::string::operator +=(const string& _str) { *this = *this + _str; return *this; }
ff::string ff::string::operator +(const char* _string) const { return (*this) + string(_string); }
bool ff::string::operator<(const ff::string& _r) const
{
	if (size() != _r.size()) { return size() < _r.size(); }
	for (uint i = 0; i < size(); i += 1) { if ((*this)[i] != _r[i]) { return (char)(*this)[i] < (char)_r[i]; } }
	return false;
}

ff::string::operator std::string() const { return std::string(data(), size()); }
ff::string::operator float() { return std::stof(*this); }

char ff::string::operator[](uint _i) const { return storage[_i]; }
char& ff::string::operator[](uint _i) { return storage[_i]; }

void ff::string::pushback(const string& _r) { *this += _r; }

void ff::string::erase(uint _idx) { storage.erase(_idx); }
void ff::string::erase(uint _idx, uint _length) { for (uint i = 0; i < _length; i += 1) { erase(_idx); } }

uint ff::string::size() const { return storage.size() - 1; }
const char* ff::string::data() const { return storage.data(); }
char* ff::string::data() { return storage.data(); }
void ff::string::mirror() { if (size() == 0) { return; } storage.popback(); storage.mirror(); storage.pushback('\0'); }

bool ff::string::find(const string& _str) const
{
	if (_str.size() > size()) { return false; }

	for (uint i = 0; i < size() - _str.size() + 1; i += 1)
	{
		bool match = true;
		for (uint j = 0; j < _str.size(); j += 1)
		{
			if ((*this)[i + j] != _str[j]) { match = false; break; }
		}
		if (match) { return true; }
	}
	return false;
}
bool ff::string::findAtIndex(const string& _str, uint _idx) const
{
	if (_str.size() + _idx > size()) { return false; }

	bool match = true;
	for (uint i = 0; i < _str.size(); i += 1)
	{
		if ((*this)[_idx + i] != _str[i]) { match = false; break; }
	}
	if (match) { return true; }
	return false;
}
uint ff::string::findIndex(const string& _str) const
{
	if (_str.size() > size()) { return size(); }

	for (uint i = 0; i < size() - _str.size() + 1; i += 1)
	{
		bool match = true;
		for (uint j = 0; j < _str.size(); j += 1)
		{
			if ((*this)[i + j] != _str[j]) { match = false; break; }
		}
		if (match) { return i; }
	}
	return size();
}

inline ff::string operator+(const char* _string1, ff::string _string2) { return ff::string(_string1) + _string2; }
std::ostream& operator<<(std::ostream& _l, const ff::string& _r) { ff::string cpy = _r; _l << cpy.data(); return _l; }

bool ff::string::saveToFile(string _fileName)
{
	std::ofstream fileStream;
	fileStream.open(_fileName);
	if (!fileStream) { return false; }
	fileStream.write(storage.data(), storage.size());
	return true;
}
uint ff::string::serialSave(ff::rawmem _target) const
{
	if (storage.size() >= 2 && storage[storage.size() - 2] == '\0' && storage[storage.size() - 1] == '\0') { std::cout << "[!] FOUND THIS SHIT!!!\n"; std::cout << storage.data() << "\n"; return 0; }
	

	if (size() + 1 > _target.size) { std::cout << "[!] FFSTRING SERIALSAVE FAILED\n"; return 0; }
	for (uint i = 0; i < size(); i += 1) { _target.address[i] = (*this)[i]; }
	_target.address[size()] = '\0';

	return size() + 1;
}
uint ff::string::getSerialSaveSize() const
{
	return size() + 1;
}
uint ff::string::serialLoad(ff::rawmem _target)
{
	if (_target.size == 0) { return 0; }
	*this = ff::string((const char*)_target.address);
	return size() + 1;
}
