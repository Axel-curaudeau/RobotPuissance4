
#pragma once

namespace ff
{
	template<typename T> struct vec2;
	typedef vec2<int> vec2i; typedef vec2<unsigned int> vec2u; typedef vec2<float> vec2f;

	template<typename T> struct vec2
	{
		T x;
		T y;

		vec2();
		vec2(T _x, T _y);
		template<typename U> vec2(const vec2<U>& _vec);

		//vec2(vec2<T> _a, vec2<T> _b) : x(_b.x - _a.x), y(_b.y - _a.y) {}
		//vec2(float _angle, float _length) : x(_length * std::cos(_angle)), y(_length * std::sin(_angle)) {}

		//template<typename U> vec2(U _x, U _y) : x((T)_x), y((T)_y) {}
		//template<typename U> vec2(vec2<U> _vec, float _length) : x((T)(_vec.x * (_length / length()))), y((T)(_vec.y * (_length / length()))) {}


		bool operator ==(const vec2<T>& _r) const;
		bool operator !=(const vec2& _r) const;
		
		vec2<T> operator +() const;
		vec2<T> operator -() const;
		vec2<T> operator +(const vec2<T>& _other) const;
		vec2<T>& operator +=(const vec2<T>& _other);
		vec2<T> operator -(const vec2<T>& _other) const;
		vec2<T>& operator -=(const vec2<T>& _other);
		template<typename U> vec2<T> operator *(const U& _scale) const;
		template<typename U> vec2<T>& operator *=(const U& _scale);
		template<typename U> vec2<T> operator /(const U& _scale) const;
		template<typename U> vec2<T>& operator /=(const U& _scale);


		vec2<T> unit() const;

		float length() const { return std::sqrtf((float)(x * x + y * y)); }
		float angle() const { return std::atan2f(y, x); } // returns angle between ]-PI, PI]

		T dotProduct(const vec2<T>& _other) const;
		T crossProduct(const vec2<T>& _other) const;


		/*vec2& rotate(float _angle)
		{
			float cos = std::cos(_angle);
			float sin = std::sin(_angle);

			float xCpy = (float)x;
			x = (T)((float)x * cos - (float)y * sin);
			y = (T)((float)xCpy * sin + (float)y * cos);

			return *this;
		}*/
	};

	//template<typename T, typename U> vec2<T> operator *(U _r, vec2<T> _l) { return _l * _r; }
	//template<typename T, typename U> vec2<T> operator /(U _r, vec2<T> _l) { return _l / _r; }

	//template<typename T> T dotProduct(vec2<T> _a, vec2<T> _b) { return _a.x * _b.x + _a.y * _b.y; } // + if _a and _b have the same direction
	//template<typename T> T dotProduct(vec2<T> _origin, vec2<T> _a, vec2<T> _b) { return (_a.x - _origin.x) * (_b.x - _origin.x) + (_a.y - _origin.y) * (_b.y - _origin.y); }
	//template<typename T> T crossProduct(vec2<T> _a, vec2<T> _b) { return _a.x * _b.y - _a.y * _b.x; } // - if _a = up(0, -1) and _b = left(-1, 0) or + if _a = up(0, -1) and _b = right(1, 0)
	//template<typename T> T crossProduct(vec2<T> _origin, vec2<T> _a, vec2<T> _b) { return (_a.x - _origin.x) * (_b.y - _origin.y) - (_a.y - _origin.y) * (_b.x - _origin.x); }
}


template<typename T> ff::vec2<T>::vec2() { x = 0; y = 0; }
template<typename T> ff::vec2<T>::vec2(T _x, T _y) { x = _x; y = _y; }
template<typename T> template<typename U> ff::vec2<T>::vec2(const vec2<U>& _other) { x = (T)_other.x; y = (T)_other.y; }
template<typename T> bool ff::vec2<T>::operator==(const vec2<T>& _other) const { return x == _other.x && y = _other.y; }
template<typename T> bool ff::vec2<T>::operator!=(const vec2<T>& _other) const { return !(*this == _other); }
template<typename T> ff::vec2<T> ff::vec2<T>::operator+() const { return *this; }
template<typename T> ff::vec2<T> ff::vec2<T>::operator-() const { return vec2<T>(-x, -y); }
template<typename T> ff::vec2<T> ff::vec2<T>::operator+(const vec2<T>& _other) const { return vec2<T>(x + _other.x, y + _other.y); }
template<typename T> ff::vec2<T>& ff::vec2<T>::operator+=(const vec2<T>& _other) { *this = *this + _other; return *this; }
template<typename T> ff::vec2<T> ff::vec2<T>::operator-(const vec2<T>& _other) const { return vec2<T>(x - _other.x, y - _other.y); }
template<typename T> ff::vec2<T>& ff::vec2<T>::operator-=(const vec2<T>& _other) { *this = *this - _other; return *this; }
template<typename T> template<typename U> ff::vec2<T> ff::vec2<T>::operator*(const U& _scale) const { return vec2<T>((T)((U)x * _scale), (T)((U)y * _scale)); }
template<typename T> template<typename U> ff::vec2<T>& ff::vec2<T>::operator*=(const U& _scale) { *this = *this * _scale; return *this; }
template<typename T> template<typename U> ff::vec2<T> ff::vec2<T>::operator/(const U& _scale) const { return vec2<T>((T)((U)x / _scale), (T)((U)y / _scale)); }
template<typename T> template<typename U> ff::vec2<T>& ff::vec2<T>::operator/=(const U& _scale) { *this = *this / _scale; return *this; }
template<typename T> ff::vec2<T> ff::vec2<T>::unit() const { return vec2<T>(x, y) / length(); }
template<typename T> T ff::vec2<T>::dotProduct(const vec2<T>& _other) const { return x * _other.x + y * _other.y; }
template<typename T> T ff::vec2<T>::crossProduct(const vec2<T>& _other) const { return x * _other.y - y * _other.x; }


