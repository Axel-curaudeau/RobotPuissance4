
#pragma once


#include "ff/ffrect.hpp"
#include "ff/ffvec2.hpp"


namespace nValueType { enum type : char { px, percent, ratio }; }
namespace nPosSide { enum type : char { topLeft = 0, botLeft = 1, topRight = 2, botRight = 3, center = 4 }; }
namespace component
{
	struct relativepos
	{
		nValueType::type target;
		ff::vec2f pos;
		ff::vec2f size;
		nPosSide::type side;

		ff::rect<int> bounds;

		relativepos() {}

		//relativepos(ff::vec2i _pos, relativeTarget::type _target, relativeSide::type _side) { pos = _pos; size = ff::vec2i(0, 0); target = _target; side = _side; }



		/// \brief 
		relativepos(nValueType::type _target, ff::vec2f _pos, ff::vec2f _size, nPosSide::type _side) { target = _target; pos = _pos; size = _size; side = _side; }

		relativepos(nValueType::type _target, float _ratioYDividedByX) { target = nValueType::ratio; pos.x = _ratioYDividedByX; }


		bool fromCenter() { return side == nPosSide::center; }
		bool fromLeft() { return ((uint)side) <= 1; }
		bool fromTop() { return ((uint)side) % 2 == 0; }


		/// \brief Update current bounds relative to the new parent bounds
		/// \param _parentBounds New parent bounds
		void update(ff::rect<int> _parentBounds);
	};
}



void component::relativepos::update(ff::rect<int> _parentBounds)
{
	if (target == nValueType::px)
	{
		if (fromCenter())
		{
			bounds.left = _parentBounds.left + _parentBounds.size().x / 2 - (int)size.x / 2 + (int)pos.x; bounds.right = _parentBounds.right + _parentBounds.size().x / 2 + (int)size.x / 2 + (int)pos.x;
			bounds.top = _parentBounds.top + _parentBounds.size().y / 2 - (int)size.y / 2 + (int)pos.y; bounds.bot = _parentBounds.bot + _parentBounds.size().y / 2 + (int)size.y / 2 + (int)pos.y;
		}
		else
		{
			if (fromLeft()) { bounds.left = _parentBounds.left + (int)pos.x; bounds.right = bounds.left + (int)size.x; }
			else { bounds.right = _parentBounds.right + (int)pos.x; bounds.left = bounds.right - (int)size.x; }
			if (fromTop()) { bounds.top = _parentBounds.top + (int)pos.y; bounds.bot = bounds.top + (int)size.y; }
			else { bounds.bot = _parentBounds.bot + (int)pos.y; bounds.top = bounds.bot - (int)size.y; }
		}
	}
	else if (target == nValueType::percent)
	{
		ff::vec2i parentSize = _parentBounds.size();
		ff::vec2i newPos = ff::vec2i((int)(pos.x * parentSize.x), (int)(pos.y * parentSize.y)); // pos has values in [0.0, 1.0] when using percent
		ff::vec2i newSize = ff::vec2i((int)(size.x * parentSize.x), (int)(size.y * parentSize.y)); // size has values in [0.0, 1.0] when using percent

		if (fromCenter())
		{
			bounds.left = _parentBounds.left + _parentBounds.size().x / 2 - newSize.x / 2 + newPos.x; bounds.right = _parentBounds.right + _parentBounds.size().x / 2 + newSize.x / 2 + newPos.x;
			bounds.top = _parentBounds.top + _parentBounds.size().y / 2 - newSize.y / 2 + newPos.y; bounds.bot = _parentBounds.bot + _parentBounds.size().y / 2 + newSize.y / 2 + newPos.y;
		}
		else
		{
			if (fromLeft()) { bounds.left = _parentBounds.left + (int)newPos.x; bounds.right = bounds.left + (int)newSize.x; }
			else { bounds.right = _parentBounds.right + (int)newPos.x; bounds.left = bounds.right - (int)newSize.x; }
			if (fromTop()) { bounds.top = _parentBounds.top + (int)newPos.y; bounds.bot = bounds.top + (int)newSize.y; }
			else { bounds.bot = _parentBounds.bot + (int)newPos.y; bounds.top = bounds.bot - (int)newSize.y; }
		}
	}
	else if (target == nValueType::ratio)
	{
		ff::vec2i parentSize = _parentBounds.size();
		ff::vec2i newPosOffset = ff::vec2i(0, 0);
		ff::vec2i newSize = parentSize;
		float ratio = pos.x;

		if (newSize.x * ratio < newSize.y) { newSize.y = (int)(newSize.x * ratio); newPosOffset.y = (parentSize.y - newSize.y) / 2; }
		else { newSize.x = (int)(newSize.y * (1.0f / ratio)); newPosOffset.x = (parentSize.x - newSize.x) / 2; }

		bounds.left = _parentBounds.left + newPosOffset.x; bounds.right = bounds.left + newSize.x;
		bounds.top = _parentBounds.top + newPosOffset.y; bounds.bot = bounds.top + newSize.y;
	}
}
