
#pragma once

#include "ffstring.hpp"
#include "ffunistring.hpp"
#include "ffcolortext.hpp"
#include <iostream>

#include <Windows.h>

namespace nLogCategory { enum type { always = 0, debug }; }

namespace ff
{
	struct log
	{
		static ff::unistring globalFilter;

		log();
		log(ff::unistring _tag);
		~log();

		bool passedFilter;

		HANDLE consoleHandle = nullptr;

		log& operator<<(const ff::unistring& _text);
		log& operator<<(const ff::colortext& _text);
		log& operator<<(const ff::color& _col);
		log& operator<<(const ff::string& _text);
		log& operator<<(const char* _text);
		template<typename T> log& operator<<(const T& _any);
	};
}





ff::unistring ff::log::globalFilter = "";


ff::log::log() { consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE); passedFilter = true; }
ff::log::log(ff::unistring _tag) { *this = log(); passedFilter = _tag.find(globalFilter); }
ff::log::~log() { SetConsoleTextAttribute(consoleHandle, 7); }

HANDLE consoleHandle = nullptr;

ff::log& ff::log::operator<<(const ff::unistring& _text)
{
	if (!passedFilter) { return *this; }
	for (uint i = 0; i < _text.size(); i += 1)
	{
		if (_text[i] < 128) { std::cout << (char)_text[i]; }
		else { std::cout << "[u" << _text[i] << "]"; }
	}
	std::cout << std::flush;
	return *this;
}
ff::log& ff::log::operator<<(const ff::colortext& _text)
{
	if (!passedFilter) { return *this; }

	if (_text.idxs.size() != _text.colors.size()) { ff::log() << "Colortext FORMAT SIZE ERROR\n"; }
	for (uint i = 0; i < _text.idxs.size(); i += 1) { if (i + 1 < _text.idxs.size() && _text.idxs[i] == _text.idxs[i + 1]) { ff::log() << "Colortext FORMAT ERROR (" << i << "th index and " << i + 1 << "th index are equal with color " << _text.colors[i].getString() << " and " << _text.colors[i + 1].getString() << ": '" << _text.text.substring(ff::interval<uint>(_text.idxs[i], _text.idxs[i] + 5).clampEnd(_text.text.size())) << "')\n"; } }

	for (uint i = 0; i < _text.idxs.size(); i += 1)
	{
		ff::color currentColor = _text.colors[i];
		currentColor.r /= 86;
		currentColor.g /= 86;
		currentColor.b /= 86;
		uint16 windowsColorCode = 0;

		if (currentColor.r >= 1) { windowsColorCode += 4; }
		if (currentColor.g >= 1) { windowsColorCode += 2; }
		if (currentColor.b >= 1) { windowsColorCode += 1; }
		if (currentColor.r == 0 && currentColor.g == 0 && currentColor.b == 0) { windowsColorCode = 8; }
		//windowsColorCode += 8; // add brightness

		SetConsoleTextAttribute(consoleHandle, windowsColorCode);

		if (_text.hasIntegrityError()) { ff::log() << "Colortext INTEGRITY ERROR\n"; return *this; }

		if (i + 1 < _text.idxs.size())
		{
			ff::log() << _text.colors[i] << _text.text.substring(ff::interval<uint>(_text.idxs[i], _text.idxs[i + 1]));
		}
		else { ff::log() << _text.colors[i] << _text.text.substring(ff::interval<uint>(_text.idxs[i], _text.text.size())); }
	}

	return *this;
}
ff::log& ff::log::operator<<(const ff::color& _col)
{
	if (!passedFilter) { return *this; }

	ff::color currentColor = _col;
	//ff::log() << "COLOR[" << _col.getString() << "] ";

	currentColor.r /= 86;
	currentColor.g /= 86;
	currentColor.b /= 86;
	uint16 windowsColorCode = 0;

	if (currentColor.r >= 1) { windowsColorCode += 4; }
	if (currentColor.g >= 1) { windowsColorCode += 2; }
	if (currentColor.b >= 1) { windowsColorCode += 1; }
	if (currentColor.r == 0 && currentColor.g == 0 && currentColor.b == 0) { windowsColorCode = 8; }
	//windowsColorCode += 8; // adds brightness


	SetConsoleTextAttribute(consoleHandle, windowsColorCode);
	return *this;
}
ff::log& ff::log::operator<<(const ff::string& _text)
{
	if (!passedFilter) { return *this; }

	std::cout << _text;
	std::cout << std::flush;
	return *this;
}
ff::log& ff::log::operator<<(const char* _text)
{
	if (!passedFilter) { return *this; }

	std::cout << _text;
	std::cout << std::flush;
	return *this;
}
template<typename T> ff::log& ff::log::operator<<(const T& _any)
{
	if (!passedFilter) { return *this; }

	std::cout << _any;
	std::cout << std::flush;
	return *this;
}



