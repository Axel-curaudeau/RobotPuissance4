
#pragma once

#include "ffdynarray.hpp"
#include "ffstring.hpp"

namespace ff
{
	template<typename KEY, typename VALUE, uint SIZE> struct hashmaparray
	{
		ff::dynarray<KEY> keys;
		ff::dynarray<VALUE> values;
		std::vector<bool> used;

		hashmaparray();

		bool contains(KEY _key) const;
		bool wouldOverwrite(KEY _key) const;
		VALUE& operator[](KEY _key);
		KEY getHashedKey(KEY _key) const;

		ff::string getString() const;


		ff::dynarray<KEY> getKeys() const;
		ff::dynarray<VALUE> getValues() const;
	};
}


template<typename KEY, typename VALUE, uint SIZE> ff::hashmaparray<KEY, VALUE, SIZE>::hashmaparray()
{
	keys.resize(SIZE);
	values.resize(SIZE);

	for (uint i = 0; i < SIZE; i += 1) { keys[i] = (KEY)i + 1; }
}
template<typename KEY, typename VALUE, uint SIZE> bool ff::hashmaparray<KEY, VALUE, SIZE>::contains(KEY _key) const { return keys[_key % SIZE] == _key; }
template<typename KEY, typename VALUE, uint SIZE> bool ff::hashmaparray<KEY, VALUE, SIZE>::wouldOverwrite(KEY _key) const { return (keys[_key % SIZE] % SIZE) == (_key % SIZE); }
template<typename KEY, typename VALUE, uint SIZE> VALUE& ff::hashmaparray<KEY, VALUE, SIZE>::operator[](KEY _key) { keys[_key % SIZE] = _key; return values[_key % SIZE]; }
template<typename KEY, typename VALUE, uint SIZE> KEY ff::hashmaparray<KEY, VALUE, SIZE>::getHashedKey(KEY _key) const { return _key % SIZE; }
template<typename KEY, typename VALUE, uint SIZE> ff::string ff::hashmaparray<KEY, VALUE, SIZE>::getString() const
{
	ff::string result = "";
	for (KEY i = 0; i < SIZE; i += 1)
	{
		if (wouldOverwrite(i)) { result += (ff::string)"Contains " + (ff::string)keys[i % SIZE] + " (hash " + (ff::string)i + ")\n"; }
	}
	return result;
}
template<typename KEY, typename VALUE, uint SIZE> ff::dynarray<KEY> ff::hashmaparray<KEY, VALUE, SIZE>::getKeys() const
{
	ff::dynarray<KEY> result;
	for (uint i = 0; i < SIZE; i += 1)
	{
		if (wouldOverwrite(i)) { result.pushback(keys[i]); }
	}
	return result;
}
template<typename KEY, typename VALUE, uint SIZE> ff::dynarray<VALUE> ff::hashmaparray<KEY, VALUE, SIZE>::getValues() const
{
	ff::dynarray<VALUE> result;
	for (uint i = 0; i < SIZE; i += 1)
	{
		if (wouldOverwrite(i)) { result.pushback(values[i]); }
	}
	return result;
}


