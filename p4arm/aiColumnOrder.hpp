
#pragma once

#include "ff/fflog.hpp"

namespace p4ai
{
	/// \brief Small storage struct / list used to store column order
	struct columnOrder
	{
		uint8 columnIdx[7] = { 255, 255, 255, 255, 255, 255, 255 };
		int8 columnScore[7] = { -1, -1, -1, -1, -1, -1, -1 };
		uint8 currentSize = 0;

		void addColumn(uint8 _columnIdx, int8 _columnScore, uint _maxColumns = 7);
		uint8 size();
		uint8 operator[](uint _idx) const;
	};
}



uint8 p4ai::columnOrder::operator[](uint _idx) const { return columnIdx[_idx]; }
void p4ai::columnOrder::addColumn(uint8 _columnIdx, int8 _columnScore, uint _maxColumns)
{
	if (size() >= _maxColumns) { return; }

	uint8 pos = currentSize;
	for (; pos > 0 && columnScore[pos - 1] < _columnScore; pos -= 1)
	{
		columnIdx[pos] = columnIdx[pos - 1];
		columnScore[pos] = columnScore[pos - 1];
	}
	columnIdx[pos] = _columnIdx;
	columnScore[pos] = _columnScore;

	currentSize += 1;
}
uint8 p4ai::columnOrder::size() { return currentSize; }


