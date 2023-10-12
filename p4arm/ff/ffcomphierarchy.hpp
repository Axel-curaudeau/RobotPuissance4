
#pragma once

#include "ffcompid.hpp"
#include "ffdynarray.hpp"

namespace ff
{
	template<typename T> struct comphierarchy
	{
		ff::dynarray<id<T>> ids;
		ff::mapdynarray<id<T>, uint> idxMap;
		ff::dynarray<ff::dynarray<id<T>>> children;
		ff::dynarray<id<T>> parents;

		void setParent(id<T> _id, id<T> _parent);
		void eraseAll();
		id<T> getParent(id<T> _id) const;
		uint getIdxInParent(id<T> _id) const;
		id<T> getPreviousSibling(id<T> _id) const;
		ff::dynarray<id<T>> getChildren(id<T> _id) const;

		/// 
		ff::mapdynarray<uint, uint> getSortMapBreadthParentsFirst(ff::dynarray<id<T>> _ids, ff::mapdynarray<id<T>, uint> _idxMap) const;

		///
		ff::mapdynarray<uint, uint> getSortMapBreadthChildrenFirst(ff::dynarray<id<T>> _ids, ff::mapdynarray<id<T>, uint> _idxMap) const;
		
		/// [!] NOT TESTED
		ff::mapdynarray<uint, uint> getSortMapDepthParentFirst(ff::dynarray<id<T>> _ids, ff::mapdynarray<id<T>, uint> _idxMap) const;

		/// \brief Get sort map where any element encountered precedes its parent
		/// \return A sort map, mapping previous idx to new idx
		ff::mapdynarray<uint, uint> getSortMapDepthChildrenFirst(ff::dynarray<id<T>> _ids, ff::mapdynarray<id<T>, uint> _idxMap) const;
	};
}

template<typename T> void ff::comphierarchy<T>::setParent(id<T> _id, id<T> _parent)
{
	if (idxMap.contains(_id)) { parents[idxMap[_id]] = _parent; }
	else { idxMap.pushback(_id, ids.size()); ids.pushback(_id); parents.pushback(_parent); children.pushback(ff::dynarray<id<T>>()); }

	if (idxMap.contains(_parent)) { children[idxMap[_parent]].pushback(_id); }
	else { idxMap.pushback(_parent, ids.size()); ids.pushback(_parent); children.pushback({ _id }); parents.pushback(id<T>()); }
}
template<typename T> ff::id<T> ff::comphierarchy<T>::getParent(id<T> _id) const
{
	if (idxMap.contains(_id)) { return parents[idxMap[_id]]; }
	else { return id<T>(); }
}
template<typename T> uint ff::comphierarchy<T>::getIdxInParent(id<T> _id) const
{
	if (idxMap.contains(_id))
	{
		ff::dynarray<id<T>> children = getChildren(getParent(_id));
		for (uint i = 0; i < children.size(); i += 1) { if (children[i] == _id) { return i; } }
	}
	else { return -1; }
}
template<typename T> ff::id<T> ff::comphierarchy<T>::getPreviousSibling(id<T> _id) const
{
	if (idxMap.contains(_id))
	{
		ff::dynarray<id<T>> children = getChildren(getParent(_id));
		if (children.size() == 1) { return ff::id<T>(); }

		if (children[0] == _id) { return ff::id<T>(); }
		for (uint i = 1; i < children.size(); i += 1) { if (children[i] == _id) { return children[i - 1]; } }
		return ff::id<T>();
	}
	else { return ff::id<T>(); }
}
template<typename T> ff::dynarray<ff::id<T>> ff::comphierarchy<T>::getChildren(id<T> _id) const
{
	if (idxMap.contains(_id)) { return children[idxMap[_id]]; }
	else { return ff::dynarray<id<T>>(); }
}



template<typename T> ff::mapdynarray<uint, uint> ff::comphierarchy<T>::getSortMapBreadthParentsFirst(ff::dynarray<id<T>> _ids, ff::mapdynarray<id<T>, uint> _idxMap) const
{
	ff::mapdynarray<uint, uint> result;

	// make a buffer that only contains elements with no parents
	ff::dynarray<id<T>> buffer;
	for (uint i = 0; i < _ids.size(); i += 1) { if (!getParent(_ids[i]).isValid()) { result.pushback(i, buffer.size()); buffer.pushback(_ids[i]); } }

	// work down to the children
	for (uint i = 0; i < buffer.size(); i += 1)
	{
		ff::dynarray<id<T>> elemChildren = getChildren(buffer[i]);
		for (uint j = 0; j < elemChildren.size(); j += 1) { if (_idxMap.contains(elemChildren[j])) { result.pushback(_idxMap[elemChildren[j]], buffer.size()); buffer.pushback(elemChildren[j]); } }
	}

	return result;
}
template<typename T> ff::mapdynarray<uint, uint> ff::comphierarchy<T>::getSortMapBreadthChildrenFirst(ff::dynarray<id<T>> _ids, ff::mapdynarray<id<T>, uint> _idxMap) const
{
	ff::mapdynarray<uint, uint> result;

	// make a buffer that only contains elements with no children
	ff::dynarray<id<T>> buffer;
	for (uint i = 0; i < _ids.size(); i += 1) { if (getChildren(_ids[i]).size() == 0) { result.pushback(i, buffer.size()); buffer.pushback(_ids[i]); } }

	// work up to the parents
	for (uint i = 0; i < buffer.size(); i += 1)
	{
		id<T> tempParent = getParent(buffer[i]);
		if (!tempParent.isValid()) { continue; }
		if (_idxMap.contains(tempParent)) { result.pushback(_idxMap[tempParent], buffer.size()); buffer.pushback(tempParent); }
	}

	return result;
}
template<typename T> ff::mapdynarray<uint, uint> ff::comphierarchy<T>::getSortMapDepthParentFirst(ff::dynarray<id<T>> _ids, ff::mapdynarray<id<T>, uint> _idxMap) const
{
	// make a buffer that only contains elements with no parents
	ff::dynarray<id<T>> buffer;
	ff::dynarray<uint> bufferOriginalIdx;
	for (uint i = 0; i < _ids.size(); i += 1) { if (!getParent(_ids[i]).isValid()) { buffer.pushback(_ids[i]); bufferOriginalIdx.pushback(i); } }

	// insert children right after itself
	for (uint i = 0; i < buffer.size(); i += 1)
	{
		ff::dynarray<id<T>> elemChildren = getChildren(buffer[i]);
		for (uint j = 0; j < elemChildren.size(); j += 1)
		{
			buffer.insert(i + 1, elemChildren[i]);
			bufferOriginalIdx.insert(i + 1, _idxMap[elemChildren[j]]);
		}
	}

	// setup original idx to new idx
	ff::mapdynarray<uint, uint> result;
	for (uint i = 0; i < buffer.size(); i += 1) { result.pushback(bufferOriginalIdx[i], i); }

	return result;
}
template<typename T> ff::mapdynarray<uint, uint> ff::comphierarchy<T>::getSortMapDepthChildrenFirst(ff::dynarray<id<T>> _ids, ff::mapdynarray<id<T>, uint> _idxMap) const
{
	ff::mapdynarray<uint, uint> result;

	uint resultIdx = 0;

	// loop only over elements with no parents (roots)
	for (uint i = 0; i < _ids.size(); i += 1)
	{
		if (getParent(_ids[i]).isValid()) { continue; }

		ff::dynarray<id<T>> stack;
		ff::dynarray<uint> stackIdx;
		ff::dynarray<uint> stackOriginalIdx;
		stack.pushback(_ids[i]); stackIdx.pushback(0); stackOriginalIdx.pushback(i);

		// explore stack
		while (stack.size() > 0)
		{
			ff::dynarray<id<T>> children = getChildren(stack.back());
			if (stackIdx.back() < children.size())
			{
				ff::id<T> child = children[stackIdx.back()];
				stackIdx.back() += 1;

				stack.pushback(child);
				stackOriginalIdx.pushback(_idxMap[child]);
				stackIdx.pushback(0);
			}
			else
			{
				stack.popback();
				stackIdx.popback();
				uint idx = stackOriginalIdx.popback();

				result.pushback(idx, resultIdx);
				resultIdx += 1;
			}
		}
	}

	return result;
}
template<typename T> void ff::comphierarchy<T>::eraseAll()
{
	idxMap.eraseAll();
	children.clear();
	parents.clear();
	ids.clear();
}