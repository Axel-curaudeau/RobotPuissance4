
#pragma once

#include "ffvec2.hpp"
#include "ffinterval.hpp"

namespace ff
{
	template<typename T> struct rect
	{
		T left = 0;
		T right = 0;
		T top = 0;
		T bot = 0;

		rect() {}
		rect(T _left, T _rightExc, T _top, T _botExc) { left = _left; right = _rightExc; top = _top; bot = _botExc; }

		ff::vec2<T> size() const { return ff::vec2<T>(right - left, bot - top); }
		T length() const { return right - left; }
		T height() const { return bot - top; }
		bool contains(ff::vec2<T> _point) { return ff::interval<T>(left, right).contains(_point.x) && ff::interval<T>(top, bot).contains(_point.y); }

		rect operator+(const ff::vec2<T> _vec) const;
		rect& operator+=(const ff::vec2<T>& _vec);

		rect operator-(const ff::vec2<T> _vec) const;
		rect& operator-=(const ff::vec2<T>& _vec);
	};
}
template<typename T> ff::rect<T> ff::rect<T>::operator+(const ff::vec2<T> _vec) const { return ff::rect<T>(left + _vec.x, right + _vec.x, top + _vec.y, bot + _vec.y); }
template<typename T> ff::rect<T>& ff::rect<T>::operator+=(const ff::vec2<T>& _vec) { *this = *this + _vec; return *this; }
template<typename T> ff::rect<T> ff::rect<T>::operator-(const ff::vec2<T> _vec) const { return ff::rect<T>(left - _vec.x, right - _vec.x, top - _vec.y, bot - _vec.y); }
template<typename T> ff::rect<T>& ff::rect<T>::operator-=(const ff::vec2<T>& _vec) { *this = *this - _vec; return *this; }


