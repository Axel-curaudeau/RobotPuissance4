
#pragma once

#include "ffvec2.hpp"

namespace ff
{
	template<typename T> struct circle;
	typedef circle<int> circlei; typedef circle<unsigned int> circleu; typedef circle<float> circlef;

	template<typename T> struct circle
	{
		vec2<T> center;
		T radius;

		circle();
		circle(vec2<T> _center, T _radius);
		circle(T _xCenter, T _yCenter, T _radius);

		bool contains(vec2<T> _point);
	};
}


template<typename T> ff::circle<T>::circle() { center = ff::vec2<T>(); radius = 0; }
template<typename T> ff::circle<T>::circle(vec2<T> _center, T _radius) { center = _center; radius = _radius; }
template<typename T> ff::circle<T>::circle(T _xCenter, T _yCenter, T _radius) { center.x = _xCenter; center.y = _yCenter; radius = _radius; }
template<typename T> bool ff::circle<T>::contains(vec2<T> _point) { double distance = (_point - center).length(); return distance <= radius; }