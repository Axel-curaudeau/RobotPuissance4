
#pragma once

#include <cmath>
#include "ffsetup.hpp"
#include "ffstring.hpp"

namespace ff
{
	struct color
	{
		uint8 r = 0;
		uint8 g = 0;
		uint8 b = 0;
		uint8 opacity = 255; // 255 = fully opaque, 0 = fully transparent

		static color rgb(uint8 _red, uint8 _green, uint8 _blue, uint8 _opacity = 255);
		static color hsv(float _hue, float _saturation, float _value, uint8 _opacity = 255); // _hue:[0.0, 360.0]   _saturation:[0.0, 1.0]   _value:[0.0, 1.0]

		bool operator == (const color& _r);
		bool operator != (const color& _r);

		void blend(const color& _col, float _ratio = 0.5f);

		ff::string getString() const;


		static color red();
		static color darkRed();
		static color lightRed();
		static color green();
		static color darkGreen();
		static color lightGreen();
		static color blue();
		static color darkBlue();
		static color lightBlue();

		static color yellow();
		static color darkYellow();
		static color lightYellow();
		static color purple();
		static color darkPurple();
		static color lightPurple();
		static color cyan();
		static color darkCyan();
		static color lightCyan();


		static color gray();
		static color darkGray();
		static color lightGray();

		static color white();
		static color black();
	};

}


ff::color ff::color::rgb(uint8 _red, uint8 _green, uint8 _blue, uint8 _opacity) { color result; result.r = _red; result.g = _green; result.b = _blue; result.opacity = _opacity; return result; }
ff::color ff::color::hsv(float _hue, float _saturation, float _value, uint8 _opacity)
{
	float chroma = _value * _saturation;

	_hue /= 60.0f;

	float x = chroma * (1.0f - std::abs(_hue - std::floor(_hue / 2.0f) * 2.0f - 1.0f));
	float diff = _value - chroma;

	float rT = 0;
	float gT = 0;
	float bT = 0;
	if (_hue < 1) { rT = chroma; gT = x; }
	else if (_hue < 2) { rT = x; gT = chroma; }
	else if (_hue < 3) { gT = chroma; bT = x; }
	else if (_hue < 4) { gT = x; bT = chroma; }
	else if (_hue < 5) { rT = x; bT = chroma; }
	else if (_hue < 6) { rT = chroma; bT = x; }
	else { rT = 0; gT = 0; bT = 0; diff = 0; }
	color result; result.r = (uint8)((rT + diff) * 255); result.g = (uint8)((gT + diff) * 255); result.b = (uint8)((bT + diff) * 255); result.opacity = _opacity; return result;
}
bool ff::color::operator ==(const color& _r) { return (r == _r.r) && (g == _r.g) && (b == _r.b) && (opacity == _r.opacity); }
bool ff::color::operator !=(const color& _r) { return (r != _r.r) || (g != _r.g) && (b != _r.b) && (opacity != _r.opacity); }
void ff::color::blend(const color& _col, float _ratio)
{
	r = (uint8)((float)r * _ratio + (float)_col.r * (1 - _ratio));
	g = (uint8)((float)g * _ratio + (float)_col.g * (1 - _ratio));
	b = (uint8)((float)b * _ratio + (float)_col.b * (1 - _ratio));
	opacity = (uint8)((float)opacity * _ratio + (float)_col.opacity * (1 - _ratio));
}
ff::string ff::color::getString() const { return (ff::string)r + "," + (ff::string)g + "," + (ff::string)b; }


ff::color ff::color::red() { return color::rgb(255, 0, 0); }
ff::color ff::color::darkRed() { return color::rgb(128, 0, 0); }
ff::color ff::color::lightRed() { return color::rgb(196, 128, 128); }
ff::color ff::color::green() { return color::rgb(0, 255, 0); }
ff::color ff::color::darkGreen() { return color::rgb(0, 128, 0); }
ff::color ff::color::lightGreen() { return color::rgb(128, 196, 128); }
ff::color ff::color::blue() { return color::rgb(0, 0, 255); }
ff::color ff::color::darkBlue() { return color::rgb(0, 0, 128); }
ff::color ff::color::lightBlue() { return color::rgb(128, 128, 196); }

ff::color ff::color::yellow() { return color::rgb(255, 255, 0); }
ff::color ff::color::darkYellow() { return color::rgb(128, 128, 0); }
ff::color ff::color::lightYellow() { return color::rgb(196, 196, 128); }
ff::color ff::color::purple() { return color::rgb(255, 0, 255); }
ff::color ff::color::darkPurple() { return color::rgb(128, 0, 128); }
ff::color ff::color::lightPurple() { return color::rgb(196, 128, 196); }
ff::color ff::color::cyan() { return color::rgb(0, 255, 255); }
ff::color ff::color::darkCyan() { return color::rgb(0, 128, 128); }
ff::color ff::color::lightCyan() { return color::rgb(128, 196, 196); }

ff::color ff::color::gray() { return color::rgb(128, 128, 128); }
ff::color ff::color::darkGray() { return color::rgb(64, 64, 64); }
ff::color ff::color::lightGray() { return color::rgb(196, 196, 196); }

ff::color ff::color::white() { return color::rgb(255, 255, 255); }
ff::color ff::color::black() { return color::rgb(0, 0, 0); }
