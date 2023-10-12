
#pragma once

#include "ff/ffstring.hpp"
#include "ff/ffcolor.hpp"
#include "ff/ffsetup.hpp"
#include "ff/ffbitops.hpp"


/// \brief Namespace used to store operations on the uint64 bitboard representation
namespace ops
{
	const uint xSize = 7;
	const uint ySize = 6;


	/// \brief Offset all cells by coordinates
	/// \param int _xDelta: x coordinate offset [-6, 6]
	/// \param int _yDelta: y coordinate offset [-5, 5]
	uint64 offset(uint64 _cells, int _xDelta, int _yDelta);

	/// \brief Get the cells surrounding the current cells
	uint64 surround(uint64 _filledCells);

	/// \brief Check if cells are aligned for a win
	bool checkWin(uint64 _cells);

	/// \brief Get the cells where dropping a token would give a win
	uint64 getWinPositions(uint64 _filledCells, uint64 _playerCells);

	/// \brief Get the cells where dropping a token is possible
	uint64 getPlaceablePositions(uint64 _filledCells);

	/// \brief Get the cells where dropping a token would give a win in the next turn
	uint64 getPlaceableWinPositions(uint64 _filledCells, uint64 _playerCells);

	/// \brief Check if dropping on a column can reach one of the cells
	/// \param uint _x: x-th column [0, 6]
	bool canReachCellWithColumn(uint64 _board, uint64 _cells, uint _x);

	/// \brief Get the cell position when dropping a column
	/// \param uint _x: x-th column [0, 6]
	uint64 getColumnDropPosition(uint64 _filledCells, uint _x);
		
	/// \brief Get a cell given its coordinates
	/// \param int _x: x coordinate of the cell [0, 6]
	/// \param int _y: y coordinate of the cell [0, 6] (warning: using the value "6" will return an invalid cell position but is allowed)
	uint64 getCellAt(uint _x, uint _y);

	/// \brief Get the string representation of the cells
	ff::string getString(uint64 _cells);
};





enum class nBoardSlot { empty = 0, firstPlayer = 1, secondPlayer = 2 };
enum class nBoardStatus { playing, firstPlayerWon, secondPlayerWon, draw, invalid };
enum class nBoardTurn { firstPlayer = 0, secondPlayer };

/// \brief Model of the board state, containing where tokens are placed and how many turns have been played (note: does NOT contain graphical elements)
struct bitboard
{
	static const uint xSize = 7;
	static const uint ySize = 6;

	uint64 p1Cells = 0;
	uint64 filledCells = 0;
	uint8 moves = 0;

	/// \brief Get the type of the cell (empty, first player, second player)
	/// \param uint _x: x coordinate of the cell [0, 6]
	/// \param uint _y: y coordinate of the cell [0, 5]
	nBoardSlot getCellType(uint _x, uint _y) const;


	void setCellType(uint _x, uint _y, nBoardSlot _type);

	/// \brief Get the score of dropping in the column
	/// \param uint _x: x coordinate of the column to check [0, 6]
	/// \return [>=1: one of the best possible moves], [0: could have won, didn't block immediate losing spot or placed token under a losing spot], [-1: impossible move]
	int8 getColumnScore(uint _x) const;

	/// \brief Check if a dropped token will be next to existing tokens
	/// \param uint _x: x coordinate of the column to check [0, 6]
	/// \return [true: the dropped token will be next to other tokens], [false]
	bool getDropIsNeighboring(uint _x) const;

	/// \brief Check if a dropped token will be next to existing allied tokens
	/// \param uint _x: x coordinate of the column to check [0, 6]
	/// \return [true: the dropped token will be next to other allied tokens], [false]
	bool getDropIsNeighboringFriendly(uint _x) const;

	/// \brief Get the state of the game (playing, first player won, second player won, draw, invalid state)
	nBoardStatus getStatus() const;

	/// \brief Get the score of the board if the game ended, or a static evaluation otherwise
	/// \return int8: Score from the point of view of the player whose turn it is to play [-100, 100]
	int8 getScore() const;

	/// \brief Check if a cell can be modified (by removing or placing a token in the spot)
	/// \param uint _x: x coordinate of the cell [0, 6]
	/// \param uint _y: y coordinate of the cell [0, 5]
	/// \return [true: if the cell can be modified]
	bool canCycle(uint _x, uint _y) const;

	/// \brief Modify a cell by placing a token if it is empty or by emptying the cell if it is filled (must call "canCycle" before to check if it can be modified)
	/// \param uint _x: x coordinate of the cell [0, 6]
	/// \param uint _y: y coordinate of the cell [0, 5]
	void cycle(uint _x, uint _y);

	/// \brief Get the predicted cell color if the cell was cycled
	/// \param uint _x: x coordinate of the cell [0, 6]
	/// \param uint _y: y coordinate of the cell [0, 5]
	/// \return [black: if the cell will be empty] [red: if the cell will have player 1's token] [yellow: if the cell will have player 2's token]
	ff::color getCycleColor(uint _x, uint _y) const;

	/// \brief Get the cell color
	/// \param uint _x: x coordinate of the cell [0, 6]
	/// \param uint _y: y coordinate of the cell [0, 5]
	/// \return [black: if the cell is empty] [red: if the cell has player 1's token] [yellow: if the cell has player 2's token]
	ff::color getCellColor(uint _x, uint _y) const;

	/// \brief Get the unique key associated with this board state
	/// \return Unique key number (NOT a representation of the board)
	uint64 getKey() const;


	bool operator==(const bitboard& _board) const;
	bool operator!=(const bitboard& _board) const;


	bool canDropColumn(uint _x);
	void dropColumn(uint _x);
	uint getTurnsLeft() const;
	uint getTurnsPlayed() const;
	nBoardTurn getTurn() const;
	ff::string getString() const;
};









uint64 ops::offset(uint64 _cells, int _xDelta, int _yDelta)
{
	uint64 result = _cells;
	if (_xDelta > 0) { result = result << (uint)_xDelta * (ops::ySize + 1); }
	else { result = result >> (uint)(-_xDelta) * (ops::ySize + 1); }
	if (_yDelta > 0) { result = result << _yDelta; }
	else { result = result >> (-_yDelta); }

	return (result) & ~(getCellAt(0, 6) + getCellAt(1, 6) + getCellAt(2, 6) + getCellAt(3, 6) + getCellAt(4, 6) + getCellAt(5, 6) + getCellAt(6, 6));
}
uint64 ops::surround(uint64 _filledCells)
{
	return (offset(_filledCells, 1, 0) | offset(_filledCells, 0, 1) | offset(_filledCells, -1, 0) | offset(_filledCells, 0, -1)) & ~_filledCells;
}
bool ops::checkWin(uint64 _cells)
{
	// Vertical
	if ((_cells & offset(_cells, 0, 1) & offset(_cells, 0, 2) & offset(_cells, 0, 3)) != 0) { return true; }
	// Horizontal
	if ((_cells & offset(_cells, 1, 0) & offset(_cells, 2, 0) & offset(_cells, 3, 0)) != 0) { return true; }
	// Diagonal up-right
	if ((_cells & offset(_cells, 1, 1) & offset(_cells, 2, 2) & offset(_cells, 3, 3)) != 0) { return true; }
	// Diagonal down-right
	if ((_cells & offset(_cells, 1, -1) & offset(_cells, 2, -2) & offset(_cells, 3, -3)) != 0) { return true; }

	return false;
}
uint64 ops::getWinPositions(uint64 _filledCells, uint64 _playerCells)
{
	uint64 result = 0;
	// Vertical
	result |= offset(_playerCells, 0, 1) & offset(_playerCells, 0, 2) & offset(_playerCells, 0, 3);
	// Horizontal
	result |= offset(_playerCells, 1, 0) & offset(_playerCells, 2, 0) & offset(_playerCells, 3, 0); // (3 in a row)
	result |= offset(_playerCells, -1, 0) & offset(_playerCells, -2, 0) & offset(_playerCells, -3, 0); // (3 in a row)
	result |= offset(_playerCells, 1, 0) & offset(_playerCells, 2, 0) & offset(_playerCells, -1, 0); // (2 in a row + 1 spaced out)
	result |= offset(_playerCells, -1, 0) & offset(_playerCells, -2, 0) & offset(_playerCells, 1, 0); // (1 spaced out + 2 in a row)
	// Diagonal up-right
	result |= offset(_playerCells, 1, 1) & offset(_playerCells, 2, 2) & offset(_playerCells, 3, 3); // (3 in a row)
	result |= offset(_playerCells, -1, -1) & offset(_playerCells, -2, -2) & offset(_playerCells, -3, -3); // (3 in a row)
	result |= offset(_playerCells, 1, 1) & offset(_playerCells, 2, 2) & offset(_playerCells, -1, -1); // (2 in a row + 1 spaced out)
	result |= offset(_playerCells, -1, -1) & offset(_playerCells, -2, -2) & offset(_playerCells, 1, 1); // (1 spaced out + 2 in a row)
	// Diagonal down-right
	result |= offset(_playerCells, 1, -1) & offset(_playerCells, 2, -2) & offset(_playerCells, 3, -3); // (3 in a row)
	result |= offset(_playerCells, -1, 1) & offset(_playerCells, -2, 2) & offset(_playerCells, -3, 3); // (3 in a row)
	result |= offset(_playerCells, 1, -1) & offset(_playerCells, 2, -2) & offset(_playerCells, -1, 1); // (2 in a row + 1 spaced out)
	result |= offset(_playerCells, -1, 1) & offset(_playerCells, -2, 2) & offset(_playerCells, 1, -1); // (1 spaced out + 2 in a row)

	return result & ~_filledCells;
}
uint64 ops::getPlaceablePositions(uint64 _filledCells)
{
	uint64 forbiddenCells = getCellAt(0, 6) + getCellAt(1, 6) + getCellAt(2, 6) + getCellAt(3, 6) + getCellAt(4, 6) + getCellAt(5, 6) + getCellAt(6, 6);
	return _filledCells + (getCellAt(0, 0) + getCellAt(1, 0) + getCellAt(2, 0) + getCellAt(3, 0) + getCellAt(4, 0) + getCellAt(5, 0) + getCellAt(6, 0)) & ~forbiddenCells;
}
uint64 ops::getPlaceableWinPositions(uint64 _filledCells, uint64 _playerCells)
{
	uint64 winPositions = getWinPositions(_filledCells, _playerCells);
	return winPositions & getPlaceablePositions(_filledCells);
}
bool ops::canReachCellWithColumn(uint64 _occupiedCells, uint64 _matchCells, uint _x)
{
	return (getColumnDropPosition(_occupiedCells, _x) & _matchCells) != 0;
}
uint64 ops::getColumnDropPosition(uint64 _filledCells, uint _x)
{
	return _filledCells + ops::getCellAt(_x, 0) & ~_filledCells;
}
uint64 ops::getCellAt(uint _x, uint _y) { return uint64(1) << _x * (ySize + 1) << _y; }
ff::string ops::getString(uint64 _cells)
{
	ff::string result = "";

	result += "~~~~~~~\n";
	for (int j = ySize - 1; j < ySize && j >= 0; j -= 1)
	{
		for (uint i = 0; i < xSize; i += 1)
		{
			if (_cells & ops::getCellAt(i, j)) { result += "O"; }
			else { result += "_"; }
		}
		result += "\n";
	}
	result += "\n";

	return result;
}






nBoardSlot bitboard::getCellType(uint _x, uint _y) const
{
	uint64 mask = ops::getCellAt(_x, _y);
	if ((mask & filledCells) == 0) { return nBoardSlot::empty; }
	else { return ((mask & p1Cells) != 0) ? nBoardSlot::firstPlayer : nBoardSlot::secondPlayer; }
}
void bitboard::setCellType(uint _x, uint _y, nBoardSlot _type)
{
	uint64 mask = ops::getCellAt(_x, _y);

	if ((_type == nBoardSlot::empty) && (getCellType(_x, _y) != nBoardSlot::empty)) { moves -= 1; }
	else if ((_type != nBoardSlot::empty) && (getCellType(_x, _y) == nBoardSlot::empty)) { moves += 1; }

	if (_type == nBoardSlot::empty) { p1Cells &= ~mask; filledCells &= ~mask; }
	else if (_type == nBoardSlot::firstPlayer) { p1Cells |= mask; filledCells |= mask; }
	else if (_type == nBoardSlot::secondPlayer) { p1Cells &= ~mask; filledCells |= mask; }
}
int8 bitboard::getColumnScore(uint _x) const
{
	if (getCellType(_x, ySize - 1) != nBoardSlot::empty) { return -1; }

	uint64 enemyBoard = p1Cells;
	uint64 selfBoard = filledCells & ~p1Cells;
	if (getTurn() == nBoardTurn::firstPlayer) { enemyBoard = selfBoard; selfBoard = p1Cells; }

	// Immediate win
	uint64 immediateWinPositions = ops::getPlaceableWinPositions(filledCells, selfBoard);
	if (immediateWinPositions != 0) { return ops::canReachCellWithColumn(filledCells, immediateWinPositions, _x) ? 100 : 0; }

	// Immediate loss
	uint64 immediateLossPositions = ops::getPlaceableWinPositions(filledCells, enemyBoard);
	if (immediateLossPositions != 0) { return ops::canReachCellWithColumn(filledCells, immediateLossPositions, _x) ? 50 : 0; }

	// Block 2-in-a-row
	uint64 enemyDoubles1 = ops::offset(enemyBoard, 1, 0) & ops::offset(enemyBoard, 2, 0) & ops::getPlaceablePositions(filledCells);
	uint64 enemyDoubles2 = ops::offset(enemyBoard, -1, 0) & ops::offset(enemyBoard, -2, 0) & ops::getPlaceablePositions(filledCells);
	if (enemyDoubles1 != 0 && enemyDoubles2 != 0) { return ops::canReachCellWithColumn(filledCells, enemyDoubles1 | enemyDoubles2, _x) ? 50 : 0; }

	// Block 2-in-a-row separated by 1 cell
	uint64 enemyTriples1 = ops::offset(enemyBoard, 1, 0) & ops::offset(enemyBoard, -1, 0) & ops::getPlaceablePositions(filledCells);
	uint64 enemyTriples2 = ops::offset(enemyTriples1, -2, 0) & ops::getPlaceablePositions(filledCells);
	uint64 enemyTriples3 = ops::offset(enemyTriples1, 2, 0) & ops::getPlaceablePositions(filledCells);

	if (enemyTriples1 != 0 && enemyTriples2 != 0 && enemyTriples3 != 0) { ops::canReachCellWithColumn(filledCells, enemyTriples1 | enemyTriples2 | enemyTriples3, _x) ? 50 : 0; }

	// Delayed loss
	uint64 delayedLossPositions = ops::getWinPositions(filledCells, enemyBoard);
	if (delayedLossPositions != 0)
	{
		if (ops::canReachCellWithColumn(filledCells, ops::offset(delayedLossPositions, 0, -1), _x)) { return 0; } // will place token under loss position
	}

	// Delayed win
	/*uint64 winPositions = ops::getWinPositions(filledCells, selfBoard + ops::getColumnDropPosition(filledCells, _x));


	int score = 0;
	score -= ff::bitops::countBits(delayedLossPositions);
	score += ff::bitops::countBits(winPositions);

	if (score <= 0) { score = 1; }*/

	return 1;
}
bool bitboard::getDropIsNeighboring(uint _x) const
{
	uint64 neighbors = ops::surround(filledCells);
	return ops::canReachCellWithColumn(filledCells, neighbors, _x);
}
bool bitboard::getDropIsNeighboringFriendly(uint _x) const
{
	uint64 selfBoard = (getTurn() == nBoardTurn::firstPlayer) ? (p1Cells) : (filledCells & ~p1Cells);
	uint64 neighbors = ops::surround(selfBoard) & ~filledCells;
	return ops::canReachCellWithColumn(filledCells, neighbors, _x);
}
nBoardStatus bitboard::getStatus() const
{
	// Check for incorrect amount of tokens:
	int p1Count = 0;
	int p2Count = 0;
	uint64 mask = uint64(1);
	for (uint i = 0; i < xSize * (ySize + 1); i += 1)
	{
		p1Count += (mask & p1Cells) != 0;
		p2Count += (mask & (filledCells & ~p1Cells)) != 0;
		mask = mask << 1;
	}
	if (p2Count > p1Count || p1Count > p2Count + 1) { return nBoardStatus::invalid; }

	// Check for "floating" tokens:
	for (uint i = 0; i < 7; i += 1)
	{
		for (int j = 1; j < 6; j += 1)
		{
			if (((filledCells & ops::getCellAt(i, j)) != 0) && ((filledCells & ops::getCellAt(i, j - 1)) == 0)) { return nBoardStatus::invalid; }
		}
	}

	// Check for win conditions:
	if (ops::checkWin(p1Cells)) { return nBoardStatus::firstPlayerWon; }
	else if (ops::checkWin(filledCells ^ p1Cells)) { return nBoardStatus::secondPlayerWon; }
	else if (p1Count + p2Count == 42) { return nBoardStatus::draw; }

	return nBoardStatus::playing;
}
int8 bitboard::getScore() const
{
	nBoardStatus gameType = getStatus();
	if (gameType == nBoardStatus::firstPlayerWon || gameType == nBoardStatus::secondPlayerWon) { return -((int)getTurnsLeft() / 2 + 1); } // if somebody won, it is now the opposite player's turn to play, therefore the score is negative
	else if (gameType == nBoardStatus::draw) { return 0; }
	return -55;
}
bool bitboard::canCycle(uint _x, uint _y) const
{
	// Slot cannot be placed if game is over
	bitboard cpy = *this;
	cpy.cycle(_x, _y);
	if (cpy.getStatus() == nBoardStatus::invalid) { return false; }
	else if (getStatus() != nBoardStatus::playing && cpy.getStatus() != nBoardStatus::playing) { return false; }
	else { return true; }
}
void bitboard::cycle(uint _x, uint _y)
{
	uint64 cell = ops::getCellAt(_x, _y);
	if ((cell & filledCells) == 0) { p1Cells |= cell * ((moves % 2) == 0); filledCells |= cell; moves += 1; } // if the cell is empty, make the current player play it
	else { p1Cells &= ~cell; filledCells &= ~cell; moves -= 1; } // if the cell is full, empty it and revert a turn
}
ff::color bitboard::getCycleColor(uint _x, uint _y) const
{
	if (getCellType(_x, _y) == nBoardSlot::firstPlayer) { return ff::color::black(); }
	else if (getCellType(_x, _y) == nBoardSlot::secondPlayer) { return ff::color::black(); }
	else
	{
		if (moves % 2 == 0) { return ff::color::red(); } // p1's turn
		else { return ff::color::yellow(); } // p2's turn
	}
}
ff::color bitboard::getCellColor(uint _x, uint _y) const
{
	if (getCellType(_x, _y) == nBoardSlot::firstPlayer) { return ff::color::red(); }
	else if (getCellType(_x, _y) == nBoardSlot::secondPlayer) { return ff::color::yellow(); }
	else { return ff::color::black(); }
}
uint64 bitboard::getKey() const
{
	return p1Cells + filledCells;
}
bool bitboard::operator==(const bitboard& _board) const
{
	return this->filledCells == _board.filledCells && this->p1Cells == _board.p1Cells;
}
bool bitboard::operator!=(const bitboard& _board) const
{
	return !((*this) == _board);
}
bool bitboard::canDropColumn(uint _x)
{
	return getCellType(_x, ySize - 1) == nBoardSlot::empty;
}
void bitboard::dropColumn(uint _x)
{
	if (!canDropColumn(_x)) { return; }

	uint64 bottomCell = ops::getCellAt(_x, 0);

	uint64 diff = (filledCells + bottomCell) & ~filledCells;
	p1Cells |= diff * (uint)(getTurn() == nBoardTurn::firstPlayer);
	filledCells |= filledCells + bottomCell;
	moves += 1;
}
uint bitboard::getTurnsLeft() const { return (xSize * ySize) - moves; }
uint bitboard::getTurnsPlayed() const { return moves; }
nBoardTurn bitboard::getTurn() const { return (moves % 2 == 0) ? nBoardTurn::firstPlayer : nBoardTurn::secondPlayer; }
ff::string bitboard::getString() const
{
	ff::string result = "";

	for (int j = ySize; j <= ySize && j >= 0; j -= 1)
	{
		for (uint i = 0; i < xSize; i += 1)
		{
			uint64 mask = ops::getCellAt(i, j);
			if (mask & p1Cells) { result += "1"; }
			else if (mask & (filledCells ^ p1Cells)) { result += "2"; }
			else { result += "_"; }
		}
		result += "\n";
	}
	result += "\n";

	return result;
}



