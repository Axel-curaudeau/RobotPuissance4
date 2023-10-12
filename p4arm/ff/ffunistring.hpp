
#pragma once

#include "ffdynarray.hpp"
#include "ffstring.hpp"
#include "ffinterval.hpp"
#include "ffrawmem.hpp"

#include <string>
#include <iostream>


namespace ff { struct unistring; struct replacement; struct file; }
namespace ff { namespace serial { ff::dynarray<byte> saveUnicodeStringUtf8(const ff::unistring& _string); ff::unistring loadUnicodeStringUtf8(byte*& _ptr); } }
namespace ff { namespace regex { struct match; } }


namespace ff
{
	struct unistring
	{
		ff::dynarray<uint32> chars;

		unistring() { chars = { '\0' }; }
		unistring(const char* _cStr) { while (_cStr[0] != '\0') { chars.pushback(_cStr[0]); _cStr += 1; } chars.pushback('\0'); }
		unistring(const char& _char) { chars = { (uint32)_char, '\0' }; }
		unistring(ff::string _str) { for (uint i = 0; i < _str.size(); i += 1) { chars.pushback(_str[i]); } chars.pushback('\0'); }
		unistring(std::string _str) { for (uint i = 0; i < _str.size(); i += 1) { chars.pushback(_str[i]); } chars.pushback('\0'); }
		unistring(std::wstring _str) { for (uint i = 0; i < _str.size(); i += 1) { chars.pushback(_str[i]); } chars.pushback('\0'); }
		unistring(const uint& _num) { if (_num == 0) { chars.pushback('0'); chars.pushback('\0'); return; } uint n = _num; while (n > 0) { chars.pushback('0' + n % 10); n /= 10; } chars.mirror(); chars.pushback('\0'); }

		operator std::string() const { std::string result; for (uint i = 0; i < size(); i += 1) { result.push_back(chars[i]); } return result; }

		bool operator==(const unistring& _other) const { if (chars.size() != _other.chars.size()) { return false; } for (uint i = 0; i < chars.size(); i += 1) { if (chars[i] != _other.chars[i]) { return false; } } return true; }
		unistring& operator +=(const unistring& _str) { *this = *this + _str; return *this; }
		unistring operator +(const unistring& _str) const { unistring result = *this; result.chars.popback(); result.chars.pushback(_str.chars); return result; }

		uint size() const { return chars.size() - 1; }
		uint32 operator[](uint _idx) const { return chars[_idx]; }
		uint32& operator[](uint _idx) { return chars[_idx]; }

		uint32 back() const;
		uint32& back();

		void erase(ff::interval<uint> _idxs) { for (uint i = 0; i < _idxs.size(); i += 1) { chars.erase(_idxs.start); } }
		void pushback(uint32 _char) { chars.back() = _char; chars.pushback('\0'); }
		void insert(unistring _string, uint _idx) { for (uint i = 0; i < _string.size(); i += 1) { chars.insert(_idx + i, _string[i]); } }
		void mirror() { chars.popback(); chars.mirror(); chars.pushback('\0'); }

		ff::dynarray<ff::regex::match> findRegex(unistring _regex);
		bool findAtIdx(unistring _match, uint _idx) { if (_match.size() + _idx > chars.size()) { return false; } for (uint i = 0; i < _match.size(); i += 1) { if (chars[_idx + i] != _match[i]) { return false; } } return true; }
		bool find(unistring _match) { if (_match.size() > chars.size()) { return false; } for (uint i = 0; i <= chars.size() - _match.size(); i += 1) { if (findAtIdx(_match, i)) { return true; } } return false; }

		void debug();

		unistring substring(ff::interval<uint> _idxs) const
		{
			unistring result;
			for (uint i = _idxs.start; i < _idxs.end; i += 1) { result.pushback(chars[i]); }
			return result;
		}

		bool saveUtf8File(ff::file _file);

		void replace(unistring _regex, replacement _replacement);
		void indent(uint _amount);

		uint serialSave(ff::rawmem _target) const;
		uint serialLoad(ff::rawmem _target);
	};

	std::ostream& operator <<(std::ostream& _l, const ff::unistring& _r) { for (uint i = 0; i < _r.size(); i += 1) { if (_r[i] < 128) { _l << (char)_r[i]; } else { _l << '[' << _r[i] << ']'; } } return _l; }
	void unistring::debug() { std::cout << "Unistring size: " << chars.size() << " -> " << *this << "\n"; }
}

namespace ff
{
	struct replacement
	{
		ff::dynarray<ff::unistring> strings;

		replacement() {}
		//replacement(ff::unistring _a, ff::unistring _b) { strings.pushback(_a); strings.pushback(_b); }
		replacement& operator<<(const unistring& _str) { strings.pushback(_str); return *this; }

	};
}

#include "ffregex.hpp"
#include "fffile.hpp"

namespace ff
{
	ff::dynarray<ff::regex::match> unistring::findRegex(unistring _regex) { return ff::regex::findMatches(*this, _regex); }
	void unistring::replace(unistring _regex, replacement _replacement)
	{
		ff::dynarray<ff::regex::match> matches = ff::regex::findMatches(*this, _regex);
		for (uint i = 0; i < matches.size(); i += 1)
		{
			erase(matches[i].idxs);
			for (uint j = i + 1; j < matches.size(); j += 1) { matches[j].idxs.start -= matches[i].idxs.size(); matches[j].idxs.end -= matches[i].idxs.size(); }

			unistring replaceStr;
			if (_replacement.strings.size() > 1)
			{
				for (uint j = 0; j < matches[i].groups.size(); j += 1)
				{
					replaceStr += _replacement.strings[j];
					replaceStr += matches[i].groups[j].text;
				}
				replaceStr += _replacement.strings.back();
			}
			else { replaceStr = _replacement.strings[0]; }

			insert(replaceStr, matches[i].idxs.start);
			for (uint j = i + 1; j < matches.size(); j += 1) { matches[j].idxs.start += replaceStr.size(); matches[j].idxs.end += replaceStr.size(); }
		}
	}
	bool unistring::saveUtf8File(ff::file _file)
	{
		ff::dynarray<byte> data = ff::serial::saveUnicodeStringUtf8(*this);
		return _file.saveBin(data);
	}
}

void ff::unistring::indent(uint _amount)
{
	ff::unistring indents;
	for (uint i = 0; i < _amount; i += 1) { indents += "\t"; }
	this->insert(indents, 0);

	for (uint i = 0; i < size(); i += 1) { if ((*this)[i] == '\n') { this->insert(indents, i + 1); } }
}

uint32 ff::unistring::back() const { if (size() == 0) { std::cout << "UNISTRING BACK ERROR\n"; } return chars[size() - 1]; }
uint32& ff::unistring::back() { if (size() == 0) { std::cout << "UNISTRING BACK ERROR REF\n"; } return chars[size() - 1]; }

uint ff::unistring::serialSave(ff::rawmem _target) const
{
	// TODO
	return 0;
}
uint ff::unistring::serialLoad(ff::rawmem _target)
{
	// TODO
	return 0;
}
