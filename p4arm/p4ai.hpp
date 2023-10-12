
#pragma once

#include "ff/ffmapdynarray.hpp"
#include "ff/ffhashmaparray.hpp"
#include "ff/fftime.hpp"
#include "ff/ffinterval.hpp"

#include "bitboard.hpp"
#include "aiBoardEvaluation.hpp"
#include "aiColumnOrder.hpp"

namespace p4ai
{
	/// \brief Hash map mapping board keys to their exhaustive evaluations
	ff::hashmaparray<uint64, boardEvaluation, 30000> hashMap;

	/// \brief Move exploration function, returns best explored move for a given board
	/// \detail You can keep calling this function repeatedly even if it did not give a finished result in the given time, because it saves exploration progress in the hashmap
	/// 
	/// \param _board: The starting position to explore
	/// \param _wantedDepth: How deep to explore for moves (more = better result but takes more time to finish)
	/// \param _timeoutMs: How much time the function is given before it times out (even if the function times out, progress is stored for the next function call)
	/// 
	/// \return The evaluation, which can be finished (exhaustive) or incomplete (aborted), and can contain a valid column to play (check with .isPlayable())
	boardEvaluation getPositionScoreNegamaxStart(bitboard _board, uint _wantedDepth, uint _timeoutMs);


	/// \brief Recursive move exploration function used by the above function
	///
	/// \param _board: Board to explore
	/// \param _window: The alpha-beta window used for alpha-beta pruning
	/// \param _maxDepth: The maximum depth to explore to
	/// \param _depth: The current depth relative to starting position
	/// \param _timeoutMs: How much time the function has until timeout is reached
	/// \param _timer: Timer used for timeout
	/// 
	/// \return The evaluation, which can be finished (exhaustive) or incomplete (aborted), and can contain a valid column to play (check with .isPlayable())
	boardEvaluation getPositionScoreNegamax(bitboard _board, ff::interval<int> _window, uint _maxDepth, uint _depth, uint _timeoutMs, const ff::timer& _timer = ff::timer());
}

p4ai::boardEvaluation p4ai::getPositionScoreNegamaxStart(bitboard _board, uint _wantedDepth, uint _timeoutMs)
{
	// Final state:
	nBoardStatus status = _board.getStatus();
	if (status == nBoardStatus::firstPlayerWon || status == nBoardStatus::secondPlayerWon || status == nBoardStatus::draw)
	{
		int score = _board.getScore();
		hashMap[_board.getKey()] = boardEvaluation(nEvaluation::exhaustive, score, 50);
		return boardEvaluation(nEvaluation::exhaustive, score, 50);
	}


	// Pruning:
	const int bestPossibleScore = 18;
	ff::interval<int> window = ff::interval<int>(-100, 101);
	window.shrinkEndToFit(bestPossibleScore);


	// Choose columns to explore:
	int8 threshold = -1;
	for (uint i = 0; i < 7; i += 1) { if (_board.getColumnScore(i) >= 1) { threshold = 0; } }
	columnOrder columns;
	uint8 colOrder[7] = { 3, 2, 4, 1, 5, 0, 6 };
	for (uint i = 0; i < 7; i += 1)
	{
		if (!_board.canDropColumn(colOrder[i])) { continue; }
		if (_board.getColumnScore(colOrder[i]) <= threshold) { continue; }
		columns.addColumn(colOrder[i], _board.getColumnScore(colOrder[i]));
	}


	// Explore possible moves:
	boardEvaluation eval = boardEvaluation();
	ff::timer timeout;
	for (uint iLoop = 0; iLoop < 1; iLoop += 1)
	{
		eval = boardEvaluation();
		window = ff::interval<int>(-100, 101);
		window.shrinkEndToFit(bestPossibleScore);

		for (uint i = 0; i < columns.size(); i += 1)
		{
			// Pruning:
			if (eval.type != nEvaluation::aborted && eval.score >= window.getMaxValue()) { continue; }

			bitboard cpy = _board;
			cpy.dropColumn(columns[i]);

			bool updated = false;
			if (hashMap.contains(cpy.getKey()) && hashMap[cpy.getKey()].type == nEvaluation::exhaustive && (uint)(hashMap[cpy.getKey()].relativeDepth + 1) >= _wantedDepth)
			{
				updated = eval.updateWithChild(hashMap[cpy.getKey()], columns[i]);
			}
			else
			{
				updated = eval.updateWithChild(getPositionScoreNegamax(cpy, ff::interval<int>(-window.end, -window.start), _wantedDepth, 1, _timeoutMs / columns.size(), ff::timer()), columns[i]);
			}

			if (updated)
			{
				window.shrinkStartToFit(eval.score + 1);
			}
		}
	}

	// Save result:
	if (eval.type == nEvaluation::exhaustive)
	{
		if (!hashMap.wouldOverwrite(_board.getKey()) || hashMap[_board.getKey()].relativeDepth < eval.relativeDepth) { hashMap[_board.getKey()] = eval; }
	}
	return eval;
}
p4ai::boardEvaluation p4ai::getPositionScoreNegamax(bitboard _board, ff::interval<int> _window, uint _maxDepth, uint _depth, uint _timeoutMs, const ff::timer& _timer)
{
	// Final state:
	nBoardStatus status = _board.getStatus();
	if (status == nBoardStatus::firstPlayerWon || status == nBoardStatus::secondPlayerWon || status == nBoardStatus::draw)
	{
		int score = _board.getScore();
		hashMap[_board.getKey()] = boardEvaluation(nEvaluation::exhaustive, score, _maxDepth - _depth);
		return boardEvaluation(nEvaluation::exhaustive, score, _maxDepth - _depth);
	}

	// Timeout & depth limit:
	if (_timer.waitedForMilli(_timeoutMs)) { return boardEvaluation(nEvaluation::aborted); }
	if (_depth >= _maxDepth) { return boardEvaluation(nEvaluation::exhaustive, 0); }

	// Pruning:
	int bestPossibleScore = (_board.getTurnsLeft() / 2);
	_window.shrinkEndToFit(bestPossibleScore);
	if (_window.size() <= 0) { return boardEvaluation(nEvaluation::exhaustive, _maxDepth - _depth); }


	// Choose columns to explore:
	int8 threshold = -1;
	for (uint i = 0; i < 7; i += 1) { if (_board.getColumnScore(i) >= 1) { threshold = 0; } }
	columnOrder columns;
	uint8 colOrder[7] = { 3, 2, 4, 1, 5, 0, 6 };
	for (uint i = 0; i < 7; i += 1)
	{
		if (_board.getColumnScore(colOrder[i]) <= threshold) { continue; }
		columns.addColumn(colOrder[i], _board.getColumnScore(colOrder[i]));
	}

	// Explore possible moves:
	boardEvaluation eval = boardEvaluation();
	for (uint i = 0; i < columns.size(); i += 1)
	{
		// Pruning:
		if (eval.type != nEvaluation::aborted && eval.score >= _window.getMaxValue()) { continue; }

		bitboard cpy = _board;
		cpy.dropColumn(columns[i]);

		bool updated = false;
		if (hashMap.contains(cpy.getKey()) && hashMap[cpy.getKey()].type == nEvaluation::exhaustive && (uint)(hashMap[cpy.getKey()].relativeDepth + 1) >= _maxDepth - _depth)
		{
			updated = eval.updateWithChild(hashMap[cpy.getKey()], columns[i]);
		}
		else
		{
			updated = eval.updateWithChild(getPositionScoreNegamax(cpy, ff::interval<int>(-_window.end, -_window.start), _maxDepth, _depth + 1, _timeoutMs, _timer), columns[i]);
		}

		if (updated)
		{
			_window.shrinkStartToFit(eval.score + 1);
		}
	}


	// Save result:
	if (eval.type == nEvaluation::exhaustive)
	{
		if (!hashMap.wouldOverwrite(_board.getKey()) || hashMap[_board.getKey()].relativeDepth < eval.relativeDepth) { hashMap[_board.getKey()] = eval; }
	}

	return eval;
}
