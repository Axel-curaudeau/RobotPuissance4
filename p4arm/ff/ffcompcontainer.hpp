
#pragma once

#include "ffcompid.hpp"
#include "ffcomphierarchy.hpp"
#include "ffdynarray.hpp"

namespace ff
{
	template<typename ID, typename T> struct compcontainer
	{
		ff::dynarray<id<ID>> ids;
		ff::dynarray<T> values;
		ff::mapdynarray<id<ID>, uint> idxMap;

		void setComponent(id<ID> _id, T _value);
		T& get(id<ID> _id);
		T get(id<ID> _id) const;
		bool contains(id<ID> _id) const;

		uint size() const;
		T operator[](uint _idx) const;
		T& operator[](uint _idx);
		id<ID> getId(uint _idx) const;

		void sortByBreadthParentsFirst(comphierarchy<ID> _hierarchy);
		void sortByBreadthChildrenFirst(comphierarchy<ID> _hierarchy);

		void sortByDepthParentsFirst(comphierarchy<ID> _hierarchy);

		/// \brief Depth sort (children first)
		void sortByDepthChildrenFirst(comphierarchy<ID> _hierarchy);
		void eraseAll();
	};
}



template<typename ID, typename T> void ff::compcontainer<ID, T>::setComponent(id<ID> _id, T _value)
{
	if (idxMap.contains(_id)) { values[idxMap[_id]] = _value; }
	else
	{
		idxMap.pushback(_id, ids.size());
		ids.pushback(_id);
		values.pushback(_value);
	}
}
template<typename ID, typename T> T& ff::compcontainer<ID, T>::get(id<ID> _id)
{
	if (!idxMap.contains(_id)) { std::cout << "[!] COMPCONTAINER Id does not exist in compcontainer\n"; }
	return values[idxMap[_id]];
}
template<typename ID, typename T> T ff::compcontainer<ID, T>::get(id<ID> _id) const
{
	if (!idxMap.contains(_id)) { std::cout << "[!] COMPCONTAINER Id does not exist in compcontainer\n"; }
	return values[idxMap[_id]];
}
template<typename ID, typename T> bool ff::compcontainer<ID, T>::contains(id<ID> _id) const
{
	return idxMap.contains(_id);
}
template<typename ID, typename T> uint ff::compcontainer<ID, T>::size() const { return values.size(); }
template<typename ID, typename T> T ff::compcontainer<ID, T>::operator[](uint _idx) const { return values[_idx]; }
template<typename ID, typename T> T& ff::compcontainer<ID, T>::operator[](uint _idx) { return values[_idx]; }
template<typename ID, typename T> ff::id<ID> ff::compcontainer<ID, T>::getId(uint _idx) const { return ids[_idx]; }
template<typename ID, typename T> void ff::compcontainer<ID, T>::sortByBreadthParentsFirst(comphierarchy<ID> _hierarchy)
{
	ff::mapdynarray<uint, uint> sortMap = _hierarchy.getSortMapBreadthParentsFirst(ids, idxMap);
	ids.sort(sortMap);
	values.sort(sortMap);
	idxMap = ids.getIdxMap();
}
template<typename ID, typename T> void ff::compcontainer<ID, T>::sortByBreadthChildrenFirst(comphierarchy<ID> _hierarchy)
{
	ff::mapdynarray<uint, uint> sortMap = _hierarchy.getSortMapBreadthChildrenFirst(ids, idxMap);
	ids.sort(sortMap);
	values.sort(sortMap);
	idxMap = ids.getIdxMap();
}
template<typename ID, typename T> void ff::compcontainer<ID, T>::sortByDepthParentsFirst(comphierarchy<ID> _hierarchy)
{
	ff::mapdynarray<uint, uint> sortMap = _hierarchy.getSortMapDepthParentFirst(ids, idxMap);
	ids.sort(sortMap);
	values.sort(sortMap);
	idxMap = ids.getIdxMap();
}
template<typename ID, typename T> void ff::compcontainer<ID, T>::sortByDepthChildrenFirst(comphierarchy<ID> _hierarchy)
{
	ff::mapdynarray<uint, uint> sortMap = _hierarchy.getSortMapDepthChildrenFirst(ids, idxMap);
	ids.sort(sortMap);
	values.sort(sortMap);
	idxMap = ids.getIdxMap();
}
template<typename ID, typename T> void ff::compcontainer<ID, T>::eraseAll()
{
	idxMap.eraseAll();
	ids.clear();
	values.clear();
}




