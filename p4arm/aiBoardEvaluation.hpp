
#pragma once

#include "ff/fflog.hpp"

namespace p4ai
{
	/// \brief Result of an evaluation (aborted, exhaustive):
	/// - aborted: the evaluation was stopped, even if a result is found partway through
	/// - exhaustive: the evaluation was fully carried out
	enum class nEvaluation : char { aborted, exhaustive };

	struct boardEvaluation
	{
		nEvaluation type = nEvaluation::exhaustive;
		uint8 relativeDepth = 0; // depth of sub-tree

		int8 score = -100;
		uint8 column = -1;

		boardEvaluation() {}
		boardEvaluation(nEvaluation _type) { type = _type; }
		boardEvaluation(nEvaluation _type, uint8 _relativeDepth) { type = _type; relativeDepth = _relativeDepth; }
		boardEvaluation(nEvaluation _type, int8 _score, uint8 _relativeDepth) { type = _type; score = _score; relativeDepth = _relativeDepth; }

		/// \brief Update the current evaluation with an evaluation of an immediate move
		/// 
		/// \param _child: The "immediate move" / "children" evaluation
		/// \param _column: The column played to get to the children state
		/// 
		/// \return [true: if the score is valid and changed] [false: otherwise]
		bool updateWithChild(boardEvaluation _child, uint8 _column);

		/// \brief Check if the move in this evaluation is playable (if the column can be played)
		///
		/// \return True if the stored column can be played, false otherwise
		bool isPlayable();

		/// \brief Get the string representation of the evaluation
		///
		/// \return "<score> by playing column <column> (<search type>)"
		ff::string getString();
	};
}



bool p4ai::boardEvaluation::updateWithChild(boardEvaluation _child, uint8 _column)
{
	if (_child.type == nEvaluation::aborted) { type = nEvaluation::aborted; }
	else
	{
		if (relativeDepth == 0) { relativeDepth = _child.relativeDepth + 1; }
		else { relativeDepth = ff::minOf(relativeDepth, _child.relativeDepth + 1); }
	}

	if (_child.score != -100)
	{
		if (score == -100 || -_child.score > score) { score = -_child.score; column = _column; return true; }
	}

	return false;
}
bool p4ai::boardEvaluation::isPlayable() { return column != (uint8)-1 && score != -100; }
ff::string p4ai::boardEvaluation::getString()
{
	ff::string result = "";

	if (score != -100)
	{
		result += (ff::string)(int)score;
		if (column != (uint8)-1) { result += " by playing column " + (ff::string)(int)column; }
	}
	else { result += "?"; }

	if (type == nEvaluation::aborted) { result += " (partial search)"; }
	else if (type == nEvaluation::exhaustive) { result += " (exhaustive search: depth " + (ff::string)relativeDepth + ")"; }

	return result;
}