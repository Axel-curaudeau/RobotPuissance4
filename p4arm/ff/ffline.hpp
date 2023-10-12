
#pragma once

#include "ffvec2.hpp"

namespace ff
{
	template<typename T> struct line;
	typedef line<int> linei; typedef line<unsigned int> lineu; typedef line<float> linef;

	template<typename T> struct line
	{
		vec2<T> point;
		vec2<T> direction;

		line();
		line(vec2<T> _point, vec2<T> _direction);

		vec2<T> getProjection(vec2<T> _point) const;
		float getDistance(vec2<T> _point) const;


	};
}


template<typename T> ff::line<T>::line() { point = ff::vec2<T>(); direction = ff::vec2<T>(); }
template<typename T> ff::line<T>::line(vec2<T> _point, vec2<T> _direction) { point = _point; direction = _direction.unit(); }
template<typename T> ff::vec2<T> ff::line<T>::getProjection(vec2<T> _point) const { return point + direction * direction.dotProduct(_point - point); }
template<typename T> float ff::line<T>::getDistance(vec2<T> _point) const { return (_point - getProjection(_point)).length(); }
