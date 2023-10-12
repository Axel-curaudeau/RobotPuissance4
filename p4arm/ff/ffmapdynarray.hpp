
#pragma once

#include <map>

#include "ffsetup.hpp"

namespace ff { template<typename T> struct dynarray; }

namespace ff
{
	template<typename Tkey, typename Uvalue> struct mapdynarray
	{
	private:
		std::map<Tkey, Uvalue> storage;

	public:
		void pushback(Tkey _key, Uvalue _value);
		void eraseAll();
		Uvalue& operator[](Tkey _key);
		Uvalue operator[](Tkey _key) const;
		ff::dynarray<Tkey> getKeys() const;
		ff::dynarray<Uvalue> getValues() const;
		bool contains(Tkey _key) const;

		uint size() const;

		ff::mapdynarray<Uvalue, Tkey> reverse() const;

	};
}

#include "ffdynarray.hpp"

template<typename T, typename U> void ff::mapdynarray<T, U>::pushback(T _key, U _value) { storage[_key] = _value; }
template<typename T, typename U> void ff::mapdynarray<T, U>::eraseAll() { storage.clear(); }
template<typename T, typename U> U& ff::mapdynarray<T, U>::operator[](T _key) { if (!contains(_key)) { std::cout << "MAPDYNARRAY ERROR []&\n"; } return storage.at(_key); }
template<typename T, typename U> U ff::mapdynarray<T, U>::operator[](T _key) const { if (!contains(_key)) { std::cout << "MAPDYNARRAY ERROR []\n"; } return storage.at(_key); }
template<typename T, typename U> ff::dynarray<T> ff::mapdynarray<T, U>::getKeys() const
{
	ff::dynarray<T> result;
	for (typename std::map<T, U>::const_iterator i = storage.begin(); i != storage.end(); ++i) { result.pushback(i->first); }
	return result;
}
template<typename T, typename U> ff::dynarray<U> ff::mapdynarray<T, U>::getValues() const
{
	ff::dynarray<U> result;
	for (typename std::map<T, U>::const_iterator i = storage.begin(); i != storage.end(); ++i) { result.pushback(i->second); }
	return result;
}
template<typename T, typename U> bool ff::mapdynarray<T, U>::contains(T _key) const { return storage.count(_key) == 1; }
template<typename T, typename U> uint ff::mapdynarray<T, U>::size() const { return (uint)storage.size(); }
template<typename T, typename U> ff::mapdynarray<U, T> ff::mapdynarray<T, U>::reverse() const { ff::mapdynarray<U, T> result; for (typename std::map<T, U>::iterator i = storage.begin(); i != storage.end(); i += 1) { result.pushback(i->second, i->first); } return result; }


