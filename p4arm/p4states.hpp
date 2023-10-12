
#pragma once


#include "p4ai.hpp"
#include "p4ui.hpp"
#include "p4dobot.hpp"
#include "p4camera.hpp"


namespace p4
{
	namespace states
	{
		enum class nState { thinking, waitingForPlayer };


		/// \brief Structure that contains all information exchanged between p4states and p4ui
		struct uiExchange { bitboard board; ff::dynarray<ff::dynarray<bool>> ammoState; bool editMode = false; };


		/// \brief Current state of the program
		nState currentState = nState::waitingForPlayer;


		/// \brief Tick function for states, call this function to attempt to change states by getting a new webcam image and checking UI
		///
		/// \param _dobot: The robot object
		/// \param _debugImg: RETURN VALUE of the debug image
		/// \param _exchange: Input and output, takes in the UI state and return the new UI state
		/// 
		/// \return The current state (waiting for player / thinking)
		nState tick(dobot& _dobot, cvimage& _debugImg, uiExchange& _exchange);
	}
}




p4::states::nState p4::states::tick(dobot& _dobot, cvimage& _debugImg, uiExchange& _exchange)
{
	bitboard imgBitboard;
	bool imgBitboardIsValid = false;

	ff::timer perf;

	cvimage img;
	if (img.getWebcamImage(0, 30) == cvimage::nWebcam::ok) // Fetch the image from webcam
	{
		img.blur(4);					   //
		static cvimage prevImg = img;	   //
		img.denoiseTemporal(prevImg, 0.1); //
		prevImg = img;					   // Remove noise from the image by blurring + averaging over time

		imgBitboardIsValid = p4cam::getBoard(img, imgBitboard, _debugImg); // Try to detect a board from the image
	}
	



	if (currentState == nState::thinking)
	{
		if (_exchange.board.getTurn() == nBoardTurn::firstPlayer) { currentState = nState::waitingForPlayer; _exchange.editMode = true; return currentState; } // If it's the player's turn to move, cancel and switch to waiting for player (this should not happen)


		p4ai::boardEvaluation eval = p4ai::getPositionScoreNegamaxStart(_exchange.board, 8, 10);														   //
		if (eval.type != p4ai::nEvaluation::exhaustive) { ff::log() << "Evaluating moves... Current: " << eval.getString() << "\n"; return currentState; } //
		if (!eval.isPlayable()) { ff::log() << "Evaluation does not provide a playable move\n"; return currentState; }									   // Evaluate moves, if evaluation is not finished return from function

		ff::log() << "Thinking finished, time to move...\n";

		_dobot.ping();
		if (_dobot.isConnected())
		{
			_dobot.setGrip(nGripStat::released); //
			_dobot.setGrip(nGripStat::off);		 // Set grabber position to open

			bool foundAmmoPos = false;
			uint ammoPosX; uint ammoPosY;
			for (uint i = 0; i < 2 && !foundAmmoPos; i += 1) { for (uint j = 0; j < 4; j += 1) { if (_exchange.ammoState[i][j]) { foundAmmoPos = true; ammoPosX = i; ammoPosY = j; break; } } }

			if (foundAmmoPos)
			{
				_dobot.moveTo(_dobot.getHighAmmoPos(ammoPosX, ammoPosY));		  //
				_dobot.moveTo(_dobot.getAmmoPos(ammoPosX, ammoPosY)); //
				std::cout << "Hello world";
				_dobot.setGrip(nGripStat::grabbing);				  //
				_dobot.moveTo(_dobot.getHighAmmoPos(ammoPosX, ammoPosY));		  // Move to the ammo and grab a token

				_dobot.moveTo(_dobot.getHighColumnPos(eval.column));			   //
				_dobot.moveTo(_dobot.getColumnPos(eval.column));	   //
				_dobot.setGrip(nGripStat::off);						   //
				ff::sleep(500);											   //
				_dobot.setGrip(nGripStat::released);

				ptpPos temp = dobot::getOffsetZColumnPos(_dobot.getColumnPos(eval.column));
				_dobot.moveTo(temp); // (<- this line is used to "bump" the token into the column)

				//
				_dobot.moveTo(dobot::getOffsetXColumnPos(temp)); // (<- this line is used to "bump" the token into the column)
				ff::sleep(500);											   //
				_dobot.moveTo(_dobot.getHighColumnPos(eval.column));			   // Move to the column and drop the token

				_dobot.setGrip(nGripStat::off);			 //
				_dobot.moveTo(_dobot.getHighAmmoPos(0,0)); // Turn grip off and place in a safer position

				_exchange.ammoState[ammoPosX][ammoPosY] = false;
			}
		}

		currentState = nState::waitingForPlayer; _exchange.board.dropColumn(eval.column); _exchange.editMode = true;
	}
	else if (currentState == nState::waitingForPlayer)
	{
		ff::log() << "Waiting for player...\n";

		// Accept new moves:
		if (imgBitboardIsValid && imgBitboard.getStatus() != nBoardStatus::invalid && imgBitboard.getTurnsPlayed() > _exchange.board.getTurnsPlayed())
		{
			_exchange.board = imgBitboard;
		}

		if (_exchange.board.getTurn() == nBoardTurn::secondPlayer)
		{
			_exchange.editMode = false;
			currentState = nState::thinking;
		}
	}

	return currentState;
}