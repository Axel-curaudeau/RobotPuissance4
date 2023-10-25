
#pragma once




#include "ff/ffcompid.hpp"
#include "ff/ffcompcontainer.hpp"
#include "ff/ffcomphierarchy.hpp"
#include "ff/ffinputstate.hpp"


#include "bitboard.hpp"
#include "uirelativepos.hpp"
#include "uidrawable.hpp"





namespace p4ui
{
	static sf::RenderWindow window;

	struct entity {};
	typedef void(*voidVoidFunction)();
	typedef bool(*clickReleaseFunction)(ff::id<entity>, ff::eventClickRelease);


	/// \brief Component id manager, used to create unique ids
	ff::compidmanager<entity> entityManager;
	/// \brief Component hierarchy, used to store information about which ids are parents of other ids
	ff::comphierarchy<entity> entityHierarchy;
	/// \brief Component container, used to store relative position per id
	ff::compcontainer<entity, component::relativepos> entityRelativePositions;
	/// \brief Component container, used to store drawables (text, rectangles, images...) per id
	ff::compcontainer<entity, component::drawable> entityDrawables;
	/// \brief Component container, used to store click events per id
	ff::compcontainer<entity, clickReleaseFunction> entityEventsClick;


	sf::Font assetFont;
	sf::Texture assetControlRestart;
	sf::Texture assetControlSettings;



	/// \brief BOARD: Model of the board, ids for each circle on the board and boolean for allowing the user to edit the board or not
	bool editMode = true;
	bitboard board;
	ff::id<entity> boardIds[7][6];


	/// \brief AMMO: ids and state
	ff::id<entity> ammoIds[2][4];
	ff::dynarray<ff::dynarray<bool>> ammoState;

	/// \brief P4 buttons: ids
	ff::id<entity> P4Ids[2];

	/// \brief BackButton: ids
	ff::id<entity> BackButtonId;

	/// \brief MainMenuButtons: ids
	ff::id<entity> mainMenuIds[2];

	/// \brief MainMenuButtons: ids
	ff::id<entity> SettingsIds[4];

	/// \brief TestConfigurationButtons: ids
	ff::id<entity> testConfigurationId;

	/// \brief AmmoLeft: ids
	ff::id<entity> ammoLeftIds[4];

	/// \brief AmmoRight: ids
	ff::id<entity> ammoRightIds[4];

	/// \brief BoardColum: ids
	ff::id<entity> boardColonIds[7];



	/// \brief CAMERA: camera sprite
	ff::id<entity> cameraId;


	/// \brief CONTROL PANEL: camera button
	ff::id<entity> controlCameraId;


	/// \brief STATE: robot state
	ff::id<entity> stateRobotId;
	ff::id<entity> stateProcessingId;

	/// \brief State of the user interface
	enum UIState
	{
		MainMenu,
		P4,
		Settings,
		CalibrationRobot,
		CalibrationCam

	};

	/// \brief store the actual state of the user interface
	static UIState uiState;

	/// \brief delete all the current UI in the window
	void deleteUI();

	/// \brief Populate the instance with the connect 4 ui elements
	void initializeP4();

	/// \brief Update the position and state of all UI elements
	/// \param _windowSize: the current size of the window in pixels
	/// \param _inputState: the current state of the inputs, containing pending events
	void updateP4(ff::vec2u _windowSize, ff::inputstate _inputState = ff::inputstate());

	/// \brief Update with a new camera frame, and the robot & program status
	/// \param _sprite: the sprite of the new camera frame (use cvimage.drawToTexture() to obtain it)
	void updateP4(ff::vec2u _windowSize, sf::Sprite _sprite, bool _robotIsConnected, bool _waitingForPlayer, ff::inputstate _inputState);

	/// \brief Update the position and state of all UI elements
	/// \param _sprite: the sprite of the new camera frame (use cvimage.drawToTexture() to obtain it)
	void update(ff::vec2u _windowSize, ff::inputstate _inputState = ff::inputstate());

	/// \brief Draw all UI elements into the window (WARNING: you MUST call update before calling this function)
	/// \param _window: The window to draw all elements in
	void draw(sf::RenderWindow& _window);

	/// \brief Populate the instance with the main menu
	void initMainMenu();

	/// \brief Update the main menu canvas with all data necessary
	void updateMainMenu(ff::vec2u _windowSize, ff::inputstate _inputState = ff::inputstate());

	/// \brief Populate the instance with the main menu
	void initSettings();

	/// \brief Update the main menu canvas with all data necessary
	void updateSettings(ff::vec2u _windowSize, ff::inputstate _inputState = ff::inputstate());

	/// \brief Populate the instance with the main menu
	void initRobotCalibration();

	/// \brief Update the main menu canvas with all data necessary
	void updateRobotCalibration(ff::vec2u _windowSize, ff::inputstate _inputState = ff::inputstate());

	/// \brief Populate the instance with the main menu
	void initCameraCalibration();

	/// \brief Update the main menu canvas with all data necessary
	void updateCameraCalibration(ff::vec2u _windowSize, sf::Sprite _sprite,ff::inputstate _inputState = ff::inputstate());

	/// \brief Update the main menu canvas with all data necessary
	void updateCameraCalibration(ff::vec2u _windowSize, ff::inputstate _inputState = ff::inputstate());
}



void p4ui::update(ff::vec2u _windowSize, ff::inputstate _inputState) {
	// Internal update: events (executes custom code if an item is clicked)
	entityEventsClick.sortByBreadthChildrenFirst(entityHierarchy);
	ff::dynarray<ff::eventClickRelease> clicks = _inputState.getClickReleaseEvents();
	for (uint i = 0; i < clicks.size(); i += 1)
	{
		for (uint j = 0; j < entityEventsClick.size(); j += 1)
		{
			ff::id<entity> id = entityEventsClick.getId(j);
			if (!entityRelativePositions.contains(id)) { ff::log() << "[!] ERROR: ID " << id.value << " has a click event component but no corresponding relative position\n"; continue; }

			component::relativepos pos = entityRelativePositions.get(id);
			if (!pos.bounds.contains(clicks[i].clickPos) || !pos.bounds.contains(clicks[i].releasePos)) { continue; }

			if (entityEventsClick[j](id, clicks[i])) { break; }
		}
	}

	// Internal update: bounds (recalculates the position of each element relative to the window or to other elements)
	entityRelativePositions.sortByBreadthParentsFirst(entityHierarchy);
	for (uint i = 0; i < entityRelativePositions.size(); i += 1)
	{
		ff::id<entity> parentId = entityHierarchy.getParent(entityRelativePositions.getId(i));
		ff::rect<int> parentBounds = ff::rect<int>(0, _windowSize.x, 0, _windowSize.y);
		if (parentId.isValid()) { parentBounds = entityRelativePositions.get(parentId).bounds; }
		entityRelativePositions[i].update(parentBounds);
	}

	// Internal update: drawables (sets the previously calculated position of all drawables)
	for (uint i = 0; i < entityDrawables.size(); i += 1)
	{
		ff::id<entity> id = entityDrawables.getId(i);
		if (!entityRelativePositions.contains(id)) { ff::log() << "[!] ERROR: ID " << id.value << " has a drawable component but no corresponding relative position\n"; continue; }

		entityDrawables[i].update(entityRelativePositions.get(id).bounds, assetFont);
	}
}

void p4ui::initializeP4()
{
	if (!assetFont.loadFromFile("assets/arial.ttf")) { std::cout << "[!] ERROR: couldn't load font 'assets/arial.ttf'\nPlease copy the asset provided folder next to the executable.\n"; ff::sleep(10000); abort(); }
	if (!assetControlRestart.loadFromFile("assets/restartButton.png")) { std::cout << "[!] ERROR: couldn't load image 'assets/restartButton.png'\nPlease copy the provided asset folder next to the executable.\n"; ff::sleep(10000); abort(); }

	uiState = p4ui::UIState::P4;
	
	ammoState.resize(2);	//
	ammoState[0].resize(4); //
	ammoState[1].resize(4); // Set correct size for ammo


	ff::id<entity> idBackButtonGirdTopLeft = entityManager.addNew();																												//
	entityDrawables.setComponent(idBackButtonGirdTopLeft, component::rect(ff::color::rgb(130, 130, 130, 255)));																		//
	entityRelativePositions.setComponent(idBackButtonGirdTopLeft, component::relativepos(nValueType::percent, ff::vec2f(0.0f, 0.0f), ff::vec2f(0.2f, 0.2f), nPosSide::topLeft));	//
																																												//
	ff::id<entity> idGridTopLeft = entityManager.addNew();																														//
	entityDrawables.setComponent(idGridTopLeft, component::rect(ff::color::rgb(130, 130, 130, 255)));																			//
	entityRelativePositions.setComponent(idGridTopLeft, component::relativepos(nValueType::percent, ff::vec2f(0.0f, 0.2f), ff::vec2f(0.2f, 0.2f), nPosSide::topLeft));			//
																																												//
	ff::id<entity> idGridTopCenter = entityManager.addNew();																													//
	entityDrawables.setComponent(idGridTopCenter, component::rect(ff::color::rgb(130, 130, 130, 255)));																			//
	entityRelativePositions.setComponent(idGridTopCenter, component::relativepos(nValueType::percent, ff::vec2f(0.2f, 0.0f), ff::vec2f(0.6f, 0.4f), nPosSide::topLeft));		//
																																												//
	ff::id<entity> idGridTopRight = entityManager.addNew();																														//
	entityDrawables.setComponent(idGridTopRight, component::rect(ff::color::rgb(130, 130, 130, 255)));																			//
	entityRelativePositions.setComponent(idGridTopRight, component::relativepos(nValueType::percent, ff::vec2f(0.8f, 0.0f), ff::vec2f(0.2f, 0.4f), nPosSide::topLeft));			//
																																												//
	ff::id<entity> idGridBotLeft = entityManager.addNew();																														//
	entityDrawables.setComponent(idGridBotLeft, component::rect(ff::color::rgb(130, 130, 130, 255)));																			//
	entityRelativePositions.setComponent(idGridBotLeft, component::relativepos(nValueType::percent, ff::vec2f(0.0f, 0.4f), ff::vec2f(0.2f, 0.6f), nPosSide::topLeft));			//
																																												//
	ff::id<entity> idGridBotCenter = entityManager.addNew();																													//
	entityDrawables.setComponent(idGridBotCenter, component::rect(ff::color::rgb(130, 130, 130, 255)));																			//
	entityRelativePositions.setComponent(idGridBotCenter, component::relativepos(nValueType::percent, ff::vec2f(0.2f, 0.4f), ff::vec2f(0.6f, 0.6f), nPosSide::topLeft));		//
																																												//
	ff::id<entity> idGridBotRight = entityManager.addNew();																														//
	entityDrawables.setComponent(idGridBotRight, component::rect(ff::color::rgb(130, 130, 130, 255)));																			//
	entityRelativePositions.setComponent(idGridBotRight, component::relativepos(nValueType::percent, ff::vec2f(0.8f, 0.4f), ff::vec2f(0.2f, 0.6f), nPosSide::topLeft));			// Divide the screen into a grid of 6



	ff::id<entity> idBoardParent = entityManager.addNew();														 //
	entityHierarchy.setParent(idBoardParent, idGridBotCenter);													 //
	entityRelativePositions.setComponent(idBoardParent, component::relativepos(nValueType::ratio, 6.0f / 7.0f)); //
	entityDrawables.setComponent(idBoardParent, component::rect(ff::color::rgb(80, 80, 80, 255)));				 // Add the board background (in top right)


	//Button back
	BackButtonId = entityManager.addNew();
	entityHierarchy.setParent(BackButtonId, idBackButtonGirdTopLeft);																									//
	entityDrawables.setComponent(BackButtonId, component::rect(ff::color::rgb(100, 100, 100, 255)));																	//
	entityRelativePositions.setComponent(BackButtonId, component::relativepos(nValueType::px, ff::vec2f(10, 10), ff::vec2f(80.0f,60.0f), nPosSide::topLeft));	//
	entityEventsClick.setComponent(BackButtonId,																														//
		[](ff::id<entity> _id, ff::eventClickRelease _event)->bool																										//
		{
			p4ui::deleteUI();
			p4ui::initMainMenu();
			p4ui::updateMainMenu(ff::vec2u(window.getSize().x, window.getSize().y));
			return true;
		}														   // Define a function for when the play button is pressed
	);
	
	// Text back button
	ff::id<entity> textButtonBack = entityManager.addNew();																													//
	entityHierarchy.setParent(textButtonBack, BackButtonId);			
	entityRelativePositions.setComponent(textButtonBack, component::relativepos(nValueType::px, ff::vec2f(10, 10), ff::vec2f(), nPosSide::topLeft));						////
	entityDrawables.setComponent(textButtonBack, component::text("Back", 24, ff::color::white()));
	
	// Add the 7x6 buttons (in board):
	const float totalCircleSpacingSize = 0.05f;
	const float totalCircleSize = 1.0f - totalCircleSpacingSize;
	for (uint i = 0; i < 7; i += 1)
	{
		for (uint j = 0; j < 6; j += 1)
		{
			ff::id<entity> idCircle = entityManager.addNew();
			entityHierarchy.setParent(idCircle, idBoardParent);
			entityRelativePositions.setComponent(idCircle, component::relativepos(nValueType::percent, ff::vec2f((totalCircleSpacingSize / 8.0f) * (i + 1) + (totalCircleSize / 7.0f) * i, (totalCircleSpacingSize / 7.0f) * (j + 1) + (totalCircleSize / 6.0f) * j), ff::vec2f(totalCircleSize / 7.0f, totalCircleSize / 6.0f), nPosSide::topLeft));
			entityDrawables.setComponent(idCircle, component::circle(ff::color::black()));
			entityEventsClick.setComponent(idCircle,
				[](ff::id<entity> _id, ff::eventClickRelease _event)->bool
				{
					uint x; uint y;
					for (uint k = 0; k < 7; k += 1) { for (uint l = 0; l < 6; l += 1) { if (boardIds[k][l] == _id) { x = k; y = l; } } }

					if (editMode && board.canCycle(x, y)) { board.cycle(x, y); }
					return true;
				}
			);

			boardIds[i][5 - j] = idCircle;
		}
	}


	//Restart Button
	ff::id<entity> idRestart = entityManager.addNew();
	entityHierarchy.setParent(idRestart, idGridTopRight);
	entityRelativePositions.setComponent(idRestart, component::relativepos(nValueType::percent, ff::vec2f(0.2f, 0.2f), ff::vec2f(0.6f, 0.6f), nPosSide::topLeft));
	entityDrawables.setComponent(idRestart, component::sprite(sf::Sprite(assetControlRestart)));
	entityEventsClick.setComponent(idRestart,
		[](ff::id<entity> _id, ff::eventClickRelease _event)->bool //
		{														   //
			board = bitboard();									   // (<- reset the board)
			return true;										   //
		}														   // Define a function for when the restart button is pressed
	);




	ff::id<entity> idAmmoParent = entityManager.addNew();														//
	entityHierarchy.setParent(idAmmoParent, idGridBotLeft);														//
	entityRelativePositions.setComponent(idAmmoParent, component::relativepos(nValueType::ratio, 5.0f / 1.0f)); //
	entityDrawables.setComponent(idAmmoParent, component::rect(ff::color::rgb(80, 80, 80, 255)));				// Add the ammo background (in bot left)

	// Add the ammo slots (in ammo):
	const float totalAmmoSpacingSize = 0.1f;
	const float totalAmmoRectSize = 1.0f - totalAmmoSpacingSize;
	for (uint i = 0; i < 2; i += 1)
	{
		for (uint j = 0; j < 4; j += 1)
		{
			ff::id<entity> idAmmo = entityManager.addNew();
			entityHierarchy.setParent(idAmmo, idAmmoParent);
			entityRelativePositions.setComponent(idAmmo, component::relativepos(nValueType::percent, ff::vec2f((totalAmmoSpacingSize / 3.0f) * (i + 1) + (totalAmmoRectSize / 2.0f) * i, (totalAmmoSpacingSize / 5.0f) * (j + 1) + (totalAmmoRectSize / 4.0f) * j), ff::vec2f(totalAmmoRectSize / 2.0f, totalAmmoRectSize / 4.0f), nPosSide::topLeft));
			entityDrawables.setComponent(idAmmo, component::rect(ff::color::rgb(255, 255, 0, 255)));

			entityEventsClick.setComponent(idAmmo,
				[](ff::id<entity> _id, ff::eventClickRelease _event)->bool
				{
					uint x; uint y;
					for (uint k = 0; k < 2; k += 1) { for (uint l = 0; l < 4; l += 1) { if (ammoIds[k][l] == _id) { x = k; y = l; } } }

					ammoState[x][y] = !ammoState[x][y];
					return true;
				}
			);

			ammoState[i][j] = true;
			ammoIds[i][j] = idAmmo;
		}
	}


	p4ui::cameraId = entityManager.addNew();																//
	entityHierarchy.setParent(cameraId, idGridTopCenter);													//
	entityRelativePositions.setComponent(cameraId, component::relativepos(nValueType::ratio, 3.0f / 4.0f)); //
	entityDrawables.setComponent(cameraId, component::sprite());											// Add the camera (in top center)


	p4ui::stateRobotId = entityManager.addNew();																								   //
	entityHierarchy.setParent(stateRobotId, idGridTopLeft);																						   //
	entityRelativePositions.setComponent(stateRobotId, component::relativepos(nValueType::px, ff::vec2f(10, 10), ff::vec2f(), nPosSide::topLeft)); //
	entityDrawables.setComponent(stateRobotId, component::text("DISCONNECTED", 12, ff::color::red()));											   // Add the "connected/disconnected" state (in top left)


	p4ui::stateProcessingId = entityManager.addNew();																									//
	entityHierarchy.setParent(stateProcessingId, idGridTopLeft);																						//
	entityRelativePositions.setComponent(stateProcessingId, component::relativepos(nValueType::px, ff::vec2f(10, 30), ff::vec2f(), nPosSide::topLeft)); //
	entityDrawables.setComponent(stateProcessingId, component::text("WAITING FOR PLAYER", 12, ff::color::white()));										// Add the "waiting for player/thinking" state (in top left)

	// Container of refill button
	ff::id<entity> ContainerRefillButton = entityManager.addNew();																																//
	entityHierarchy.setParent(ContainerRefillButton, idGridBotRight);																												//
	entityRelativePositions.setComponent(ContainerRefillButton, component::relativepos(nValueType::ratio, 1.0f/1.0f));					//

	//Refill button
	P4Ids[0] = entityManager.addNew();
	entityHierarchy.setParent(P4Ids[0], ContainerRefillButton);																									//
	entityDrawables.setComponent(P4Ids[0], component::rect(ff::color::rgb(150, 150, 0, 255)));																	//
	entityRelativePositions.setComponent(P4Ids[0], component::relativepos(nValueType::percent, ff::vec2f(-0.2f,0.0f), ff::vec2f(0.7f, 0.7f), nPosSide::topRight));			//
	entityEventsClick.setComponent(P4Ids[0],																														//
		[](ff::id<entity> _id, ff::eventClickRelease _event)->bool																										//
		{
			for (uint i = 0; i < 2; i += 1)
			{
				for (uint j = 0; j < 4; j += 1)
				{
					ammoState[i][j] = true;
				}
			}
			return true;
		}														   // Define a function for when the play button is pressed
	);

	// Text refill button
	ff::id<entity> textRefillButton = entityManager.addNew();																													//
	entityHierarchy.setParent(textRefillButton, P4Ids[0]);																												//
	entityRelativePositions.setComponent(textRefillButton, component::relativepos(nValueType::px, ff::vec2f(-30, -20), ff::vec2f(), nPosSide::center));						//
	entityDrawables.setComponent(textRefillButton, component::text("Refill", 25, ff::color::white()));


}
void p4ui::updateP4(ff::vec2u _windowSize, ff::inputstate _inputState)
{
	ff::color col1 = ff::color::rgb(100, 100, 100, 255);// color change for the play button
	if (entityRelativePositions.get(BackButtonId).bounds.contains(_inputState.mousePosition))
	{
		col1 = ff::color::rgb(64, 64, 64, 255);
	}
	entityDrawables.setComponent(BackButtonId, component::rect(col1));

	// Update the board graphics
	for (uint i = 0; i < 7; i += 1)
	{
		for (uint j = 0; j < 6; j += 1)
		{
			ff::color col = board.getCellColor(i, j);
			if (entityRelativePositions.get(boardIds[i][j]).bounds.contains(_inputState.mousePosition))
			{
				if (board.canCycle(i, j) && editMode) { col.blend(board.getCycleColor(i, j), 0.75f); }
				else { col = ff::color::darkGray(); }
			}

			entityDrawables.setComponent(boardIds[i][j], component::circle(col));
		}
	}

	// Update the ammo graphics
	for (uint i = 0; i < 2; i += 1)
	{
		for (uint j = 0; j < 4; j += 1)
		{
			if (ammoState[i][j]) { entityDrawables.setComponent(ammoIds[i][j], component::rect(ff::color::rgb(255, 255, 0, 196))); }
			else { entityDrawables.setComponent(ammoIds[i][j], component::rect(ff::color::rgb(64, 64, 0, 196))); }
		}
	}

	update(_windowSize, _inputState);
}
void p4ui::updateP4(ff::vec2u _windowSize, sf::Sprite _sprite, bool _robotIsConnected, bool _waitingForPlayer, ff::inputstate _inputState)
{
	entityDrawables.get(cameraId) = component::drawable(_sprite); // Update camera sprite

	if (_robotIsConnected) { entityDrawables.get(stateRobotId) = component::text("CONNECTED", 12, ff::color::green()); } //
	else { entityDrawables.get(stateRobotId) = component::text("DISCONNECTED", 12, ff::color::red()); }					 // Update robot "disconnected/connected" text

	if (_waitingForPlayer) { entityDrawables.get(stateProcessingId) = component::text("WAITING FOR PLAYER", 12, ff::color::white()); } //
	else { entityDrawables.get(stateProcessingId) = component::text("THINKING & PLAYING", 12, ff::color::purple()); }				   // Update program "waiting for player/thinking" text

	updateP4(_windowSize, _inputState);
}
void p4ui::draw(sf::RenderWindow& _window)
{
	for (uint i = 0; i < entityDrawables.size(); i += 1) { entityDrawables[i].draw(_window); }
}

void p4ui::initMainMenu() {
	if (!assetControlSettings.loadFromFile("assets/Settings.png")) { std::cout << "[!] ERROR: couldn't load image 'assets/restartButton.png'\nPlease copy the provided asset folder next to the executable.\n"; ff::sleep(10000); abort(); }
	if (!assetFont.loadFromFile("assets/arial.ttf")) { std::cout << "[!] ERROR: couldn't load font 'assets/arial.ttf'\nPlease copy the asset provided folder next to the executable.\n"; ff::sleep(10000); abort(); }
	
	uiState = p4ui::UIState::MainMenu;

	ff::id<entity> idGridTop = entityManager.addNew();																														//
	entityDrawables.setComponent(idGridTop, component::rect(ff::color::rgb(130, 130, 130, 255)));																			//
	entityRelativePositions.setComponent(idGridTop, component::relativepos(nValueType::percent, ff::vec2f(0.0f, 0.0f), ff::vec2f(1.0f, 0.5f), nPosSide::topLeft));			//


	ff::id<entity> idGridCenter = entityManager.addNew();																													//
	entityDrawables.setComponent(idGridCenter, component::rect(ff::color::rgb(130, 130, 130, 255)));																		//
	entityRelativePositions.setComponent(idGridCenter, component::relativepos(nValueType::percent, ff::vec2f(0.0f, 0.5f), ff::vec2f(1.0f, 0.4f), nPosSide::topLeft));		//

	mainMenuIds[1] = entityManager.addNew();																																//
	entityDrawables.setComponent(mainMenuIds[1], component::rect(ff::color::rgb(130, 130, 130, 255)));																		//
	entityRelativePositions.setComponent(mainMenuIds[1], component::relativepos(nValueType::percent, ff::vec2f(0.9f, 0.89f), ff::vec2f(0.1f, 0.11f), nPosSide::topLeft));		//

	ff::id<entity> idGridBottomLeft = entityManager.addNew();																												//
	entityDrawables.setComponent(idGridBottomLeft, component::rect(ff::color::rgb(130, 130, 130, 255)));																	//
	entityRelativePositions.setComponent(idGridBottomLeft, component::relativepos(nValueType::percent, ff::vec2f(0.0f, 0.8f), ff::vec2f(0.9f, 0.2f), nPosSide::topLeft));	//


	// Container of play button
	ff::id<entity> ContainerPlayButton = entityManager.addNew();																																//
	entityHierarchy.setParent(ContainerPlayButton, idGridCenter);																												//
	entityRelativePositions.setComponent(ContainerPlayButton, component::relativepos(nValueType::px, ff::vec2f(), ff::vec2f(), nPosSide::center));					//

	//Play button
	mainMenuIds[0] = entityManager.addNew();																																//
	entityHierarchy.setParent(mainMenuIds[0], ContainerPlayButton);																												//
	entityDrawables.setComponent(mainMenuIds[0], component::rect(ff::color::rgb(100, 100, 100, 255)));																		//
	entityRelativePositions.setComponent(mainMenuIds[0], component::relativepos(nValueType::px, ff::vec2f(-70,-35), ff::vec2f(140, 70), nPosSide::topLeft));					//
	entityEventsClick.setComponent(mainMenuIds[0],
		[](ff::id<entity> _id, ff::eventClickRelease _event)->bool
		{
			p4ui::deleteUI();
			p4ui::initializeP4();
			p4ui::updateP4(ff::vec2u(window.getSize().x, window.getSize().y));
			return true;
		}														   // Define a function for when the play button is pressed
	);

	//Text Play
	ff::id<entity> textButton = entityManager.addNew();																													//
	entityHierarchy.setParent(textButton, mainMenuIds[0]);																												//
	entityRelativePositions.setComponent(textButton, component::relativepos(nValueType::px, ff::vec2f(-20,-20), ff::vec2f(), nPosSide::center));	//
	entityDrawables.setComponent(textButton, component::text("Play", 24, ff::color::white()));																			//

	//Text Puissance 4
	ff::id<entity> textCenter = entityManager.addNew();																													//
	entityHierarchy.setParent(textCenter, idGridTop);																													//
	entityRelativePositions.setComponent(textCenter, component::relativepos(nValueType::px, ff::vec2f(-230, -20), ff::vec2f(), nPosSide::center));	//
	entityDrawables.setComponent(textCenter, component::text("Puissance 4", 82, ff::color::white()));																	//


	//Button Options
	ff::id<entity> idOptions = entityManager.addNew();																													//
	entityHierarchy.setParent(idOptions, mainMenuIds[1]);																												//
	entityRelativePositions.setComponent(idOptions, component::relativepos(nValueType::ratio, 6.0f / 7.0f));		//
	entityDrawables.setComponent(idOptions, component::sprite(sf::Sprite(assetControlSettings)));																		//
	entityEventsClick.setComponent(idOptions,
		[](ff::id<entity> _id, ff::eventClickRelease _event)->bool //
		{														   //
			p4ui::deleteUI();									   //
			initSettings();										   //
			updateSettings(ff::vec2u(window.getSize().x, window.getSize().y));				   //
			return true;										   //
		}														   // Define a function for when the option button is pressed
	);

}


void p4ui::updateMainMenu(ff::vec2u _windowSize, ff::inputstate _inputState) {

	ff::color col1 = ff::color::rgb(100, 100, 100, 255);											// color change for the play button
	if (entityRelativePositions.get(mainMenuIds[0]).bounds.contains(_inputState.mousePosition))		//
	{																								//		
		col1 = ff::color::rgb(64, 64, 64, 255);														//
	}																								//
	entityDrawables.setComponent(mainMenuIds[0], component::rect(col1));							//
																									
	ff::color col2 = ff::color::rgb(130, 130, 130, 255);											//color change for the setting button
	if (entityRelativePositions.get(mainMenuIds[1]).bounds.contains(_inputState.mousePosition))		//
	{																								//
		col2 = ff::color::rgb(64, 64, 64, 255);														//
	}																								//
	entityDrawables.setComponent(mainMenuIds[1], component::rect(col2));							//

	update(_windowSize, _inputState);
}

void p4ui::initSettings() {
	if (!assetFont.loadFromFile("assets/arial.ttf")) { std::cout << "[!] ERROR: couldn't load font 'assets/arial.ttf'\nPlease copy the asset provided folder next to the executable.\n"; ff::sleep(10000); abort(); }

	uiState = p4ui::UIState::Settings;

	ff::id<entity> idBackButtonGirdTopLeft = entityManager.addNew();																												//
	entityDrawables.setComponent(idBackButtonGirdTopLeft, component::rect(ff::color::rgb(130, 130, 130, 255)));																		//
	entityRelativePositions.setComponent(idBackButtonGirdTopLeft, component::relativepos(nValueType::percent, ff::vec2f(0.0f, 0.0f), ff::vec2f(0.2f, 0.2f), nPosSide::topLeft));	//

	ff::id<entity> idGridLeft = entityManager.addNew();																													//
	entityDrawables.setComponent(idGridLeft, component::rect(ff::color::rgb(130, 130, 130, 255)));																		//
	entityRelativePositions.setComponent(idGridLeft, component::relativepos(nValueType::percent, ff::vec2f(0.0f, 0.2f), ff::vec2f(0.2f, 0.8f), nPosSide::topLeft));		//

	ff::id<entity> idGridRight = entityManager.addNew();																												//
	entityDrawables.setComponent(idGridRight, component::rect(ff::color::rgb(130, 130, 130, 255)));																		//
	entityRelativePositions.setComponent(idGridRight, component::relativepos(nValueType::percent, ff::vec2f(0.2f, 0.0f), ff::vec2f(0.8f, 1.0f), nPosSide::topLeft));	//


	//Button back
	BackButtonId = entityManager.addNew();
	entityHierarchy.setParent(BackButtonId, idBackButtonGirdTopLeft);																									//
	entityDrawables.setComponent(BackButtonId, component::rect(ff::color::rgb(100, 100, 100, 255)));																	//
	entityRelativePositions.setComponent(BackButtonId, component::relativepos(nValueType::px, ff::vec2f(10, 10), ff::vec2f(80.0f, 60.0f), nPosSide::topLeft));			//
	entityEventsClick.setComponent(BackButtonId,																														//
		[](ff::id<entity> _id, ff::eventClickRelease _event)->bool																										//
		{
			p4ui::deleteUI();
			p4ui::initMainMenu();
			p4ui::updateMainMenu(ff::vec2u(window.getSize().x, window.getSize().y));
			return true;
		}														   // Define a function for when the play back is pressed
	);

	// Text back button
	ff::id<entity> textButtonBack = entityManager.addNew();																													//
	entityHierarchy.setParent(textButtonBack, BackButtonId);																												//
	entityRelativePositions.setComponent(textButtonBack, component::relativepos(nValueType::px, ff::vec2f(10, 10), ff::vec2f(), nPosSide::topLeft));						//
	entityDrawables.setComponent(textButtonBack, component::text("Back", 24, ff::color::white()));


	//Button Calibrage Robot
	SettingsIds[0] = entityManager.addNew();
	entityHierarchy.setParent(SettingsIds[0], idGridRight);																									//
	entityDrawables.setComponent(SettingsIds[0], component::rect(ff::color::rgb(100, 100, 100, 255)));																	//
	entityRelativePositions.setComponent(SettingsIds[0], component::relativepos(nValueType::percent, ff::vec2f(0.0f, 0.2f), ff::vec2f(0.8f, 0.1f), nPosSide::topLeft));			//
	entityEventsClick.setComponent(SettingsIds[0],																														//
		[](ff::id<entity> _id, ff::eventClickRelease _event)->bool																										//
		{
			p4ui::deleteUI();
			p4ui::initRobotCalibration();
			p4ui::updateRobotCalibration(ff::vec2u(window.getSize().x, window.getSize().y));
			return true;
		}														   // Define a function for when the play button is pressed
	);

	// Text Calibrage position button
	ff::id<entity> textButtonCalibragePosition = entityManager.addNew();																													//
	entityHierarchy.setParent(textButtonCalibragePosition, SettingsIds[0]);																												//
	entityRelativePositions.setComponent(textButtonCalibragePosition, component::relativepos(nValueType::px, ff::vec2f(10, 10), ff::vec2f(), nPosSide::topLeft));						//
	entityDrawables.setComponent(textButtonCalibragePosition, component::text("Calibration Robot", 24, ff::color::white()));

	//Button Calibrage Camera
	SettingsIds[1] = entityManager.addNew();
	entityHierarchy.setParent(SettingsIds[1], idGridRight);																									//
	entityDrawables.setComponent(SettingsIds[1], component::rect(ff::color::rgb(100, 100, 100, 255)));																	//
	entityRelativePositions.setComponent(SettingsIds[1], component::relativepos(nValueType::percent, ff::vec2f(0.0f, 0.35f), ff::vec2f(0.8f, 0.1f), nPosSide::topLeft));			//
	entityEventsClick.setComponent(SettingsIds[1],																														//
		[](ff::id<entity> _id, ff::eventClickRelease _event)->bool																										//
		{
			p4ui::deleteUI();
			p4ui::initCameraCalibration();
			return true;
		}														   // Define a function for when the play button is pressed
	);

	// Text Calibrage position button
	ff::id<entity> textButtonCalibrageCam = entityManager.addNew();																													//
	entityHierarchy.setParent(textButtonCalibrageCam, SettingsIds[1]);																												//
	entityRelativePositions.setComponent(textButtonCalibrageCam, component::relativepos(nValueType::px, ff::vec2f(10, 10), ff::vec2f(), nPosSide::topLeft));						//
	entityDrawables.setComponent(textButtonCalibrageCam, component::text("Calibration Camera", 24, ff::color::white()));


	//Button Connect to robot
	SettingsIds[2] = entityManager.addNew();
	entityHierarchy.setParent(SettingsIds[2], idGridRight);																									//
	entityDrawables.setComponent(SettingsIds[2], component::rect(ff::color::rgb(100, 100, 100, 255)));																	//
	entityRelativePositions.setComponent(SettingsIds[2], component::relativepos(nValueType::percent, ff::vec2f(0.0f, 0.5f), ff::vec2f(0.8f, 0.1f), nPosSide::topLeft));			//
	entityEventsClick.setComponent(SettingsIds[2],																														//
		[](ff::id<entity> _id, ff::eventClickRelease _event)->bool																										//
		{
			//bot.connect();
			return true;
		}														   // Define a function for when the play button is pressed
	);

	// Text Connect to robot
	ff::id<entity> textButtonConnectRobot = entityManager.addNew();																													//
	entityHierarchy.setParent(textButtonConnectRobot, SettingsIds[2]);																												//
	entityRelativePositions.setComponent(textButtonConnectRobot, component::relativepos(nValueType::px, ff::vec2f(10,10), ff::vec2f(), nPosSide::topLeft));						//
	entityDrawables.setComponent(textButtonConnectRobot, component::text("Connect to robot", 24, ff::color::white()));

	//Button ChangeColor
	SettingsIds[3] = entityManager.addNew();
	entityHierarchy.setParent(SettingsIds[3], idGridRight);																									//
	entityDrawables.setComponent(SettingsIds[3], component::rect(ff::color::rgb(100, 100, 100, 255)));																	//
	entityRelativePositions.setComponent(SettingsIds[3], component::relativepos(nValueType::percent, ff::vec2f(0.0f, 0.65f), ff::vec2f(0.8f, 0.1f), nPosSide::topLeft));			//
	entityEventsClick.setComponent(SettingsIds[3],																														//
		[](ff::id<entity> _id, ff::eventClickRelease _event)->bool																										//
		{
			return true;
		}														   // Define a function for when the play button is pressed
	);

	// Text ChangeColor
	ff::id<entity> textButtonChangeColor = entityManager.addNew();																													//
	entityHierarchy.setParent(textButtonChangeColor, SettingsIds[3]);																												//
	entityRelativePositions.setComponent(textButtonChangeColor, component::relativepos(nValueType::px, ff::vec2f(10,10), ff::vec2f(), nPosSide::topLeft));						//
	entityDrawables.setComponent(textButtonChangeColor, component::text("Change Player Color", 24, ff::color::white()));


	ff::id<entity> ColorRect = entityManager.addNew();
	entityHierarchy.setParent(ColorRect, SettingsIds[3]);																															//
	entityDrawables.setComponent(ColorRect, component::rect(ff::color::rgb(255, 255, 0, 60)));																				//
	entityRelativePositions.setComponent(ColorRect, component::relativepos(nValueType::percent, ff::vec2f(-0.005f,0.05f), ff::vec2f(0.1f, 0.9f), nPosSide::topRight));
}

void p4ui::updateSettings(ff::vec2u _windowSize, ff::inputstate _inputState) {

	ff::color col1 = ff::color::rgb(100, 100, 100, 255);// color change for the play button
	if (entityRelativePositions.get(BackButtonId).bounds.contains(_inputState.mousePosition))
	{
		col1 = ff::color::rgb(64, 64, 64, 255);
	}
	entityDrawables.setComponent(BackButtonId, component::rect(col1));
	
	for (int i = 0; i < 4; i++) {
		ff::color col1 = ff::color::rgb(100, 100, 100, 255);// color change for the play button
		if (entityRelativePositions.get(SettingsIds[i]).bounds.contains(_inputState.mousePosition))
		{
			col1 = ff::color::rgb(64, 64, 64, 255);
		}
		entityDrawables.setComponent(SettingsIds[i], component::rect(col1));
	}
	
	update(_windowSize, _inputState);
}

void p4ui::initRobotCalibration() {
	if (!assetFont.loadFromFile("assets/arial.ttf"))
	{
		std::cout << "[!] ERROR: couldn't load font 'assets/arial.ttf'\nPlease copy the asset provided folder next to the executable.\n";
		ff::sleep(10000);
		abort();
	}

	uiState = p4ui::UIState::CalibrationRobot;

	ff::id<entity> idBackButtonGirdTopLeft = entityManager.addNew();																												//
	entityDrawables.setComponent(idBackButtonGirdTopLeft, component::rect(ff::color::rgb(130, 130, 130, 255)));																		//
	entityRelativePositions.setComponent(idBackButtonGirdTopLeft, component::relativepos(nValueType::percent, ff::vec2f(0.0f, 0.0f), ff::vec2f(0.2f, 0.2f), nPosSide::topLeft));	//
	
	ff::id<entity> idGridLeft = entityManager.addNew();																													//
	entityDrawables.setComponent(idGridLeft, component::rect(ff::color::rgb(130, 130, 130, 255)));																		//
	entityRelativePositions.setComponent(idGridLeft, component::relativepos(nValueType::percent, ff::vec2f(0.0f, 0.2f), ff::vec2f(0.15f, 0.6f), nPosSide::topLeft));		//

	ff::id<entity> idBackButtonGirdBottomLeft = entityManager.addNew();																												//
	entityDrawables.setComponent(idBackButtonGirdBottomLeft, component::rect(ff::color::rgb(130, 130, 130, 255)));																		//
	entityRelativePositions.setComponent(idBackButtonGirdBottomLeft, component::relativepos(nValueType::percent, ff::vec2f(0.0f, 0.8f), ff::vec2f(0.2f, 0.2f), nPosSide::topLeft));	//

	ff::id<entity> idGirdTopRight = entityManager.addNew();																												//
	entityDrawables.setComponent(idGirdTopRight, component::rect(ff::color::rgb(130, 130, 130, 255)));																		//
	entityRelativePositions.setComponent(idGirdTopRight, component::relativepos(nValueType::percent, ff::vec2f(0.85f, 0.0f), ff::vec2f(0.2f, 0.2f), nPosSide::topLeft));	//

	ff::id<entity> idGridRight = entityManager.addNew();																													//
	entityDrawables.setComponent(idGridRight, component::rect(ff::color::rgb(130, 130, 130, 255)));																		//
	entityRelativePositions.setComponent(idGridRight, component::relativepos(nValueType::percent, ff::vec2f(0.85f, 0.2f), ff::vec2f(0.15f, 0.6f), nPosSide::topLeft));		//

	ff::id<entity> idGirdBottomRight = entityManager.addNew();																												//
	entityDrawables.setComponent(idGirdBottomRight, component::rect(ff::color::rgb(130, 130, 130, 255)));																		//
	entityRelativePositions.setComponent(idGirdBottomRight, component::relativepos(nValueType::percent, ff::vec2f(0.85f, 0.8f), ff::vec2f(0.2f, 0.2f), nPosSide::topLeft));	//

	ff::id<entity> idGridCenterTop = entityManager.addNew();																												//
	entityDrawables.setComponent(idGridCenterTop, component::rect(ff::color::rgb(130, 130, 130, 255)));																		//
	entityRelativePositions.setComponent(idGridCenterTop, component::relativepos(nValueType::percent, ff::vec2f(0.15f, 0.0f), ff::vec2f(0.7f, 0.7f), nPosSide::topLeft));	//

	ff::id<entity> idGridCenterBottom = entityManager.addNew();																												//
	entityDrawables.setComponent(idGridCenterBottom, component::rect(ff::color::rgb(130, 130, 130, 255)));																		//
	entityRelativePositions.setComponent(idGridCenterBottom, component::relativepos(nValueType::percent, ff::vec2f(0.15f, 0.7f), ff::vec2f(0.7f, 0.3f), nPosSide::topLeft));	//


		//Button back
	BackButtonId = entityManager.addNew();
	entityHierarchy.setParent(BackButtonId, idBackButtonGirdTopLeft);																									//
	entityDrawables.setComponent(BackButtonId, component::rect(ff::color::rgb(100, 100, 100, 255)));																	//
	entityRelativePositions.setComponent(BackButtonId, component::relativepos(nValueType::px, ff::vec2f(10, 10), ff::vec2f(80.0f, 60.0f), nPosSide::topLeft));			//
	entityEventsClick.setComponent(BackButtonId,																														//
		[](ff::id<entity> _id, ff::eventClickRelease _event)->bool																										//
		{
			p4ui::deleteUI();
			p4ui::initSettings();
			p4ui::updateSettings(ff::vec2u(window.getSize().x, window.getSize().y));
			return true;
		}														   // Define a function for when the play back is pressed
	);

	// Text back button
	ff::id<entity> textButtonBack = entityManager.addNew();																													//
	entityHierarchy.setParent(textButtonBack, BackButtonId);																												//
	entityRelativePositions.setComponent(textButtonBack, component::relativepos(nValueType::px, ff::vec2f(10, 10), ff::vec2f(), nPosSide::topLeft));						//
	entityDrawables.setComponent(textButtonBack, component::text("Back", 24, ff::color::white()));


	ff::id<entity> idBoardParent = entityManager.addNew();														 //
	entityHierarchy.setParent(idBoardParent, idGridCenterBottom);													 //
	entityRelativePositions.setComponent(idBoardParent, component::relativepos(nValueType::ratio, 1.0f / 7.0f)); //
	entityDrawables.setComponent(idBoardParent, component::rect(ff::color::rgb(80, 80, 80, 255)));				 // Add the board background (in top right)

	// Create board position
	const float totalCircleSpacingSize = 0.05f;
	const float totalCircleSize = 1.0f - totalCircleSpacingSize;
	for (uint i = 0; i < 7; i += 1)
	{
		ff::id<entity> idCircle = entityManager.addNew();
		entityHierarchy.setParent(idCircle, idBoardParent);
		entityRelativePositions.setComponent(idCircle, component::relativepos(nValueType::percent, ff::vec2f((totalCircleSpacingSize / 8.0f) * (i + 1) + (totalCircleSize / 7.0f) * i, 0.0f), ff::vec2f(totalCircleSize / 7.0f, totalCircleSize / 1.0f), nPosSide::topLeft));
		entityDrawables.setComponent(idCircle, component::circle(ff::color::yellow()));
		entityEventsClick.setComponent(idCircle,
			[](ff::id<entity> _id, ff::eventClickRelease _event)->bool
			{
				return true;
			}
		);

		boardColonIds[i] = idCircle;
	}

	ff::id<entity> idAmmoLeftParent = entityManager.addNew();														 //
	entityHierarchy.setParent(idAmmoLeftParent, idGridLeft);													 //
	entityRelativePositions.setComponent(idAmmoLeftParent, component::relativepos(nValueType::ratio, 5.0f / 1.0f)); //
	entityDrawables.setComponent(idAmmoLeftParent, component::rect(ff::color::rgb(80, 80, 80, 255)));				 // Add the board background (in top right)

	for (uint i = 0; i < 4; i += 1)
	{
		ff::id<entity> idCircle = entityManager.addNew();
		entityHierarchy.setParent(idCircle, idAmmoLeftParent);
		entityRelativePositions.setComponent(idCircle, component::relativepos(nValueType::percent, ff::vec2f((totalCircleSpacingSize / 1.0f) * 1, (totalCircleSpacingSize / 1.0f) * (i + 1) + (totalCircleSize / 5.0f) * i), ff::vec2f(totalCircleSize / 1.0f, totalCircleSize / 5.0f), nPosSide::topLeft));
		entityDrawables.setComponent(idCircle, component::circle(ff::color::yellow()));
		entityEventsClick.setComponent(idCircle,
			[](ff::id<entity> _id, ff::eventClickRelease _event)->bool
			{
				std::cout << "bouton " << _id;
				
				return true;
			}
		);

		ammoLeftIds[i] = idCircle;
	}

	ff::id<entity> idAmmoRightParent = entityManager.addNew();														 //
	entityHierarchy.setParent(idAmmoRightParent, idGridRight);													 //
	entityRelativePositions.setComponent(idAmmoRightParent, component::relativepos(nValueType::ratio, 5.0f / 1.0f)); //
	entityDrawables.setComponent(idAmmoRightParent, component::rect(ff::color::rgb(80, 80, 80, 255)));				 // Add the board background (in top right)

	for (uint i = 0; i < 4; i += 1)
	{
		ff::id<entity> idCircle = entityManager.addNew();
		entityHierarchy.setParent(idCircle, idAmmoRightParent);
		entityRelativePositions.setComponent(idCircle, component::relativepos(nValueType::percent, ff::vec2f((totalCircleSpacingSize / 1.0f) * 1, (totalCircleSpacingSize / 1.0f) * (i + 1) + (totalCircleSize / 5.0f) * i), ff::vec2f(totalCircleSize / 1.0f, totalCircleSize / 5.0f), nPosSide::topLeft));
		entityDrawables.setComponent(idCircle, component::circle(ff::color::yellow()));
		entityEventsClick.setComponent(idCircle,
			[](ff::id<entity> _id, ff::eventClickRelease _event)->bool
			{
				std::cout << "bouton " << _id;
				
				return true;
			}
		);

		ammoRightIds[i]= idCircle;
	}

	// Container of testConfiguration button
	ff::id<entity> ContainerTestConfigurationButton = entityManager.addNew();																																//
	entityHierarchy.setParent(ContainerTestConfigurationButton, idGridCenterTop);																												//
	entityRelativePositions.setComponent(ContainerTestConfigurationButton, component::relativepos(nValueType::px, ff::vec2f(), ff::vec2f(), nPosSide::center));					//

	//testConfiguration button
	testConfigurationId = entityManager.addNew();																																//
	entityHierarchy.setParent(testConfigurationId, ContainerTestConfigurationButton);																												//
	entityDrawables.setComponent(testConfigurationId, component::rect(ff::color::rgb(100, 100, 100, 255)));																		//
	entityRelativePositions.setComponent(testConfigurationId, component::relativepos(nValueType::px, ff::vec2f(-140, -35), ff::vec2f(280, 70), nPosSide::topLeft));					//
	entityEventsClick.setComponent(testConfigurationId,
		[](ff::id<entity> _id, ff::eventClickRelease _event)->bool
		{
			std::cout << "FINI LE BOUTON (828 p4ui)";
			return true;
		}														   // Define a function for when the play button is pressed
	);

	//Text testConfiguration
	ff::id<entity> textButton = entityManager.addNew();																													//
	entityHierarchy.setParent(textButton, testConfigurationId);																												//
	entityRelativePositions.setComponent(textButton, component::relativepos(nValueType::px, ff::vec2f(-100, -20), ff::vec2f(), nPosSide::center));	//
	entityDrawables.setComponent(textButton, component::text("Test Configuration", 24, ff::color::white()));

}

void p4ui::updateRobotCalibration(ff::vec2u _windowSize, ff::inputstate _inputState) {

	ff::color col1 = ff::color::rgb(100, 100, 100, 255);// color change for the play button
	if (entityRelativePositions.get(BackButtonId).bounds.contains(_inputState.mousePosition))
	{
		col1 = ff::color::rgb(64, 64, 64, 255);
	}
	entityDrawables.setComponent(testConfigurationId, component::rect(col1));

	ff::color col2 = ff::color::rgb(100, 100, 100, 255);// color change for the play button
	if (entityRelativePositions.get(testConfigurationId).bounds.contains(_inputState.mousePosition))
	{
		col2 = ff::color::rgb(64, 64, 64, 255);
	}
	entityDrawables.setComponent(testConfigurationId, component::rect(col2));

	ff::color col3 = ff::color::rgb(255, 255, 0, 255);// color change for the play button
	for (int i = 0; i < 4; i++) {
		if (entityRelativePositions.get(ammoLeftIds[i]).bounds.contains(_inputState.mousePosition))
		{
			col3 = ff::color::rgb(255, 255, 100, 255);
		}
		entityDrawables.setComponent(ammoLeftIds[i], component::circle(col3));
	}

	for (int i = 0; i < 4; i++) {
		if (entityRelativePositions.get(ammoRightIds[i]).bounds.contains(_inputState.mousePosition))
		{
			col3 = ff::color::rgb(255, 255, 100, 255);
		}
		entityDrawables.setComponent(ammoRightIds[i], component::circle(col3));
	}

	for (int i = 0; i < 7; i++) {
		if (entityRelativePositions.get(boardColonIds[i]).bounds.contains(_inputState.mousePosition))
		{
			col3 = ff::color::rgb(255, 255, 100, 255);
		}
		entityDrawables.setComponent(boardColonIds[i], component::circle(col3));
	}
	update(_windowSize, _inputState);
}

void p4ui::initCameraCalibration() {
	if (!assetFont.loadFromFile("assets/arial.ttf")) { std::cout << "[!] ERROR: couldn't load font 'assets/arial.ttf'\nPlease copy the asset provided folder next to the executable.\n"; ff::sleep(10000); abort(); }

	uiState = p4ui::UIState::CalibrationCam;

	ff::id<entity> idBackButtonGirdTopLeft = entityManager.addNew();																												//
	entityDrawables.setComponent(idBackButtonGirdTopLeft, component::rect(ff::color::rgb(130, 130, 130, 255)));																		//
	entityRelativePositions.setComponent(idBackButtonGirdTopLeft, component::relativepos(nValueType::percent, ff::vec2f(0.0f, 0.0f), ff::vec2f(0.2f, 0.2f), nPosSide::topLeft));	//

	ff::id<entity> idGridTop = entityManager.addNew();																													//
	entityDrawables.setComponent(idGridTop, component::rect(ff::color::rgb(130, 130, 130, 255)));																		//
	entityRelativePositions.setComponent(idGridTop, component::relativepos(nValueType::percent, ff::vec2f(0.2f, 0.0f), ff::vec2f(0.7f, 0.2f), nPosSide::topLeft));		//

	ff::id<entity> idGridLeft = entityManager.addNew();																													//
	entityDrawables.setComponent(idGridLeft, component::rect(ff::color::rgb(130, 130, 130, 255)));																		//
	entityRelativePositions.setComponent(idGridLeft, component::relativepos(nValueType::percent, ff::vec2f(0.0f, 0.2f), ff::vec2f(0.7f, 0.8f), nPosSide::topLeft));		//

	ff::id<entity> idGridRight = entityManager.addNew();																													//
	entityDrawables.setComponent(idGridRight, component::rect(ff::color::rgb(130, 130, 130, 255)));																		//
	entityRelativePositions.setComponent(idGridRight, component::relativepos(nValueType::percent, ff::vec2f(0.7f, 0.0f), ff::vec2f(0.3f, 1.0f), nPosSide::topLeft));		//


	//Button back
	BackButtonId = entityManager.addNew();
	entityHierarchy.setParent(BackButtonId, idBackButtonGirdTopLeft);																									//
	entityDrawables.setComponent(BackButtonId, component::rect(ff::color::rgb(100, 100, 100, 255)));																	//
	entityRelativePositions.setComponent(BackButtonId, component::relativepos(nValueType::px, ff::vec2f(10, 10), ff::vec2f(80.0f, 60.0f), nPosSide::topLeft));			//
	entityEventsClick.setComponent(BackButtonId,																														//
		[](ff::id<entity> _id, ff::eventClickRelease _event)->bool																										//
		{
			p4ui::deleteUI();
			p4ui::initSettings();
			p4ui::updateSettings(ff::vec2u(window.getSize().x, window.getSize().y));
			return true;
		}// Define a function for when the back button is pressed
	);

	// Text back button
	ff::id<entity> textButtonBack = entityManager.addNew();																													//
	entityHierarchy.setParent(textButtonBack, BackButtonId);																												//
	entityRelativePositions.setComponent(textButtonBack, component::relativepos(nValueType::px, ff::vec2f(10, 10), ff::vec2f(), nPosSide::topLeft));						//
	entityDrawables.setComponent(textButtonBack, component::text("Back", 24, ff::color::white()));

	// Container of Camera
	ff::id<entity> ContainerCamera = entityManager.addNew();																									//
	entityHierarchy.setParent(ContainerCamera, idGridLeft);	
	entityRelativePositions.setComponent(ContainerCamera, component::relativepos(nValueType::percent, ff::vec2f(0.05f,0.05f), ff::vec2f(0.8f,0.8f), nPosSide::topLeft));					//

	// Camera on the left
	p4ui::cameraId = entityManager.addNew();																//
	entityHierarchy.setParent(cameraId, ContainerCamera);													//
	entityRelativePositions.setComponent(cameraId, component::relativepos(nValueType::ratio, 3.0f / 4.0f)); //
	entityDrawables.setComponent(cameraId, component::sprite());											// Add the camera (on the left)

	//------------------------------------------------------------------------------------------------------------------------------------------

	//text SliderMinMax
	ff::id<entity> SLiderTextMin1 = entityManager.addNew();																													//
	entityHierarchy.setParent(SLiderTextMin1, idGridRight);																												//
	entityRelativePositions.setComponent(SLiderTextMin1, component::relativepos(nValueType::percent, ff::vec2f(0.05f, 0.01f), ff::vec2f(0.8f, 0.05f), nPosSide::topLeft));
	entityDrawables.setComponent(SLiderTextMin1, component::text("Max Jeuton Rouge", 12, ff::color::white()));

	// Container of SliderH
	ff::id<entity> ContainerSliderH = entityManager.addNew();																									//
	entityHierarchy.setParent(ContainerSliderH, idGridRight);
	entityRelativePositions.setComponent(ContainerSliderH, component::relativepos(nValueType::percent, ff::vec2f(0.05f, 0.05f), ff::vec2f(0.8f, 0.05f), nPosSide::topLeft));

	//Button sliderH
	ff::id<entity> SliderH= entityManager.addNew();
	entityHierarchy.setParent(SliderH, ContainerSliderH);																									//
	entityDrawables.setComponent(SliderH, component::rect(ff::color::rgb(170, 170, 170, 255)));																	//
	entityRelativePositions.setComponent(SliderH, component::relativepos(nValueType::px, ff::vec2f(), ff::vec2f(150, 10.0f), nPosSide::topLeft));			//

	//knob sliderH
	ff::id<entity> SliderKnobH = entityManager.addNew();
	entityHierarchy.setParent(SliderKnobH, SliderH);																									//
	entityDrawables.setComponent(SliderKnobH, component::circle(ff::color::rgb(100, 100, 100, 255)));																	//
	entityRelativePositions.setComponent(SliderKnobH, component::relativepos(nValueType::px, ff::vec2f(5,-5), ff::vec2f(20, 20), nPosSide::topRight));			//

	//text SliderH
	ff::id<entity> SLiderTextH = entityManager.addNew();																													//
	entityHierarchy.setParent(SLiderTextH, ContainerSliderH);																												//
	entityRelativePositions.setComponent(SLiderTextH, component::relativepos(nValueType::px, ff::vec2f(-5, -5), ff::vec2f(), nPosSide::topRight));						//
	entityDrawables.setComponent(SLiderTextH, component::text("H : 360", 12, ff::color::white()));

	// Container of SliderS
	ff::id<entity> ContainerSliderS = entityManager.addNew();																									//
	entityHierarchy.setParent(ContainerSliderS, idGridRight);
	entityRelativePositions.setComponent(ContainerSliderS, component::relativepos(nValueType::percent, ff::vec2f(0.05f, 0.1f), ff::vec2f(0.8f, 0.05f), nPosSide::topLeft));

	//Button sliderS
	ff::id<entity> SliderS = entityManager.addNew();
	entityHierarchy.setParent(SliderS, ContainerSliderS);																									//
	entityDrawables.setComponent(SliderS, component::rect(ff::color::rgb(170, 170, 170, 255)));																	//
	entityRelativePositions.setComponent(SliderS, component::relativepos(nValueType::px, ff::vec2f(), ff::vec2f(150, 10.0f), nPosSide::topLeft));			//

	//knob sliderH
	ff::id<entity> SliderKnobS = entityManager.addNew();
	entityHierarchy.setParent(SliderKnobS, SliderS);																									//
	entityDrawables.setComponent(SliderKnobS, component::circle(ff::color::rgb(100, 100, 100, 255)));																	//
	entityRelativePositions.setComponent(SliderKnobS, component::relativepos(nValueType::px, ff::vec2f(5, -5), ff::vec2f(20, 20), nPosSide::topRight));			//

	//text SliderH
	ff::id<entity> SLiderTextS = entityManager.addNew();																													//
	entityHierarchy.setParent(SLiderTextS, ContainerSliderS);																												//
	entityRelativePositions.setComponent(SLiderTextS, component::relativepos(nValueType::px, ff::vec2f(-5, -5), ff::vec2f(), nPosSide::topRight));						//
	entityDrawables.setComponent(SLiderTextS, component::text("S : 100", 12, ff::color::white()));

	// Container of SliderV
	ff::id<entity> ContainerSliderV = entityManager.addNew();																									//
	entityHierarchy.setParent(ContainerSliderV, idGridRight);
	entityRelativePositions.setComponent(ContainerSliderV, component::relativepos(nValueType::percent, ff::vec2f(0.05f, 0.15f), ff::vec2f(0.8f, 0.05f), nPosSide::topLeft));

	//Button SliderV
	ff::id<entity> SliderV = entityManager.addNew();
	entityHierarchy.setParent(SliderV, ContainerSliderV);																									//
	entityDrawables.setComponent(SliderV, component::rect(ff::color::rgb(170, 170, 170, 255)));																	//
	entityRelativePositions.setComponent(SliderV, component::relativepos(nValueType::px, ff::vec2f(), ff::vec2f(150, 10.0f), nPosSide::topLeft));			//

	//knob SliderV
	ff::id<entity> SliderKnobV = entityManager.addNew();
	entityHierarchy.setParent(SliderKnobV, SliderV);																									//
	entityDrawables.setComponent(SliderKnobV, component::circle(ff::color::rgb(100, 100, 100, 255)));																	//
	entityRelativePositions.setComponent(SliderKnobV, component::relativepos(nValueType::px, ff::vec2f(5, -5), ff::vec2f(20, 20), nPosSide::topRight));			//

	//text SliderV
	ff::id<entity> SLiderTextV = entityManager.addNew();																													//
	entityHierarchy.setParent(SLiderTextV, ContainerSliderV);																												//
	entityRelativePositions.setComponent(SLiderTextV, component::relativepos(nValueType::px, ff::vec2f(-5, -5), ff::vec2f(), nPosSide::topRight));						//
	entityDrawables.setComponent(SLiderTextV, component::text("V : 100", 12, ff::color::white()));

	//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//text SliderMinMax
	ff::id<entity> SLiderTextMin2 = entityManager.addNew();																													//
	entityHierarchy.setParent(SLiderTextMin2, idGridRight);																												//
	entityRelativePositions.setComponent(SLiderTextMin2, component::relativepos(nValueType::percent, ff::vec2f(0.05f, 0.25f), ff::vec2f(0.8f, 0.05f), nPosSide::topLeft));
	entityDrawables.setComponent(SLiderTextMin2, component::text("Min Jeuton Rouge", 12, ff::color::white()));

	// Container of SliderH
	ff::id<entity> ContainerSliderH1 = entityManager.addNew();																									//
	entityHierarchy.setParent(ContainerSliderH1, idGridRight);
	entityRelativePositions.setComponent(ContainerSliderH1, component::relativepos(nValueType::percent, ff::vec2f(0.05f, 0.3f), ff::vec2f(0.8f, 0.05f), nPosSide::topLeft));

	//Button sliderH
	ff::id<entity> SliderH1 = entityManager.addNew();
	entityHierarchy.setParent(SliderH1, ContainerSliderH1);																									//
	entityDrawables.setComponent(SliderH1, component::rect(ff::color::rgb(170, 170, 170, 255)));																	//
	entityRelativePositions.setComponent(SliderH1, component::relativepos(nValueType::px, ff::vec2f(), ff::vec2f(150, 10.0f), nPosSide::topLeft));			//

	//knob sliderH
	ff::id<entity> SliderKnobH1 = entityManager.addNew();
	entityHierarchy.setParent(SliderKnobH1, SliderH1);																									//
	entityDrawables.setComponent(SliderKnobH1, component::circle(ff::color::rgb(100, 100, 100, 255)));																	//
	entityRelativePositions.setComponent(SliderKnobH1, component::relativepos(nValueType::px, ff::vec2f(5, -5), ff::vec2f(20, 20), nPosSide::topRight));			//

	//text SliderH
	ff::id<entity> SLiderTextH1 = entityManager.addNew();																													//
	entityHierarchy.setParent(SLiderTextH1, ContainerSliderH1);																												//
	entityRelativePositions.setComponent(SLiderTextH1, component::relativepos(nValueType::px, ff::vec2f(-5, -5), ff::vec2f(), nPosSide::topRight));						//
	entityDrawables.setComponent(SLiderTextH1, component::text("H : 360", 12, ff::color::white()));

	// Container of SliderS
	ff::id<entity> ContainerSliderS1 = entityManager.addNew();																									//
	entityHierarchy.setParent(ContainerSliderS1, idGridRight);
	entityRelativePositions.setComponent(ContainerSliderS1, component::relativepos(nValueType::percent, ff::vec2f(0.05f, 0.35f), ff::vec2f(0.8f, 0.05f), nPosSide::topLeft));

	//Button sliderS
	ff::id<entity> SliderS1 = entityManager.addNew();
	entityHierarchy.setParent(SliderS1, ContainerSliderS1);																									//
	entityDrawables.setComponent(SliderS1, component::rect(ff::color::rgb(170, 170, 170, 255)));																	//
	entityRelativePositions.setComponent(SliderS1, component::relativepos(nValueType::px, ff::vec2f(), ff::vec2f(150, 10.0f), nPosSide::topLeft));			//

	//knob sliderH
	ff::id<entity> SliderKnobS1 = entityManager.addNew();
	entityHierarchy.setParent(SliderKnobS1, SliderS1);																									//
	entityDrawables.setComponent(SliderKnobS1, component::circle(ff::color::rgb(100, 100, 100, 255)));																	//
	entityRelativePositions.setComponent(SliderKnobS1, component::relativepos(nValueType::px, ff::vec2f(5, -5), ff::vec2f(20, 20), nPosSide::topRight));			//

	//text SliderH
	ff::id<entity> SLiderTextS1 = entityManager.addNew();																													//
	entityHierarchy.setParent(SLiderTextS1, ContainerSliderS1);																												//
	entityRelativePositions.setComponent(SLiderTextS1, component::relativepos(nValueType::px, ff::vec2f(-5, -5), ff::vec2f(), nPosSide::topRight));						//
	entityDrawables.setComponent(SLiderTextS1, component::text("S : 100", 12, ff::color::white()));

	// Container of SliderV
	ff::id<entity> ContainerSliderV1 = entityManager.addNew();																									//
	entityHierarchy.setParent(ContainerSliderV1, idGridRight);
	entityRelativePositions.setComponent(ContainerSliderV1, component::relativepos(nValueType::percent, ff::vec2f(0.05f, 0.40f), ff::vec2f(0.8f, 0.05f), nPosSide::topLeft));

	//Button SliderV
	ff::id<entity> SliderV1 = entityManager.addNew();
	entityHierarchy.setParent(SliderV1, ContainerSliderV1);																									//
	entityDrawables.setComponent(SliderV1, component::rect(ff::color::rgb(170, 170, 170, 255)));																	//
	entityRelativePositions.setComponent(SliderV1, component::relativepos(nValueType::px, ff::vec2f(), ff::vec2f(150, 10.0f), nPosSide::topLeft));			//

	//knob SliderV
	ff::id<entity> SliderKnobV1 = entityManager.addNew();
	entityHierarchy.setParent(SliderKnobV1, SliderV1);																									//
	entityDrawables.setComponent(SliderKnobV1, component::circle(ff::color::rgb(100, 100, 100, 255)));																	//
	entityRelativePositions.setComponent(SliderKnobV1, component::relativepos(nValueType::px, ff::vec2f(5, -5), ff::vec2f(20, 20), nPosSide::topRight));			//

	//text SliderV
	ff::id<entity> SLiderTextV1 = entityManager.addNew();																													//
	entityHierarchy.setParent(SLiderTextV1, ContainerSliderV1);																												//
	entityRelativePositions.setComponent(SLiderTextV1, component::relativepos(nValueType::px, ff::vec2f(-5, -5), ff::vec2f(), nPosSide::topRight));						//
	entityDrawables.setComponent(SLiderTextV1, component::text("V : 100", 12, ff::color::white()));

	//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//text SliderMinMax
	ff::id<entity> SLiderTextMin3 = entityManager.addNew();																													//
	entityHierarchy.setParent(SLiderTextMin3, idGridRight);																												//
	entityRelativePositions.setComponent(SLiderTextMin3, component::relativepos(nValueType::percent, ff::vec2f(0.05f, 0.50f), ff::vec2f(0.8f, 0.05f), nPosSide::topLeft));
	entityDrawables.setComponent(SLiderTextMin3, component::text("Max Jeuton Jaune", 12, ff::color::white()));

	// Container of SliderH2
	ff::id<entity> ContainerSliderH22 = entityManager.addNew();																									//
	entityHierarchy.setParent(ContainerSliderH22, idGridRight);
	entityRelativePositions.setComponent(ContainerSliderH22, component::relativepos(nValueType::percent, ff::vec2f(0.05f, 0.55f), ff::vec2f(0.8f, 0.05f), nPosSide::topLeft));

	//Button SliderH2
	ff::id<entity> SliderH2 = entityManager.addNew();
	entityHierarchy.setParent(SliderH2, ContainerSliderH22);																									//
	entityDrawables.setComponent(SliderH2, component::rect(ff::color::rgb(170, 170, 170, 255)));																	//
	entityRelativePositions.setComponent(SliderH2, component::relativepos(nValueType::px, ff::vec2f(), ff::vec2f(150, 10.0f), nPosSide::topLeft));			//

	//knob SliderH2
	ff::id<entity> SliderKnobH2 = entityManager.addNew();
	entityHierarchy.setParent(SliderKnobH2, SliderH2);																									//
	entityDrawables.setComponent(SliderKnobH2, component::circle(ff::color::rgb(100, 100, 100, 255)));																	//
	entityRelativePositions.setComponent(SliderKnobH2, component::relativepos(nValueType::px, ff::vec2f(5, -5), ff::vec2f(20, 20), nPosSide::topRight));			//

	//text SliderH2
	ff::id<entity> SLiderTextH2 = entityManager.addNew();																													//
	entityHierarchy.setParent(SLiderTextH2, ContainerSliderH22);																												//
	entityRelativePositions.setComponent(SLiderTextH2, component::relativepos(nValueType::px, ff::vec2f(-5, -5), ff::vec2f(), nPosSide::topRight));						//
	entityDrawables.setComponent(SLiderTextH2, component::text("H : 360", 12, ff::color::white()));

	// Container of SliderS2
	ff::id<entity> ContainerSliderS2 = entityManager.addNew();																									//
	entityHierarchy.setParent(ContainerSliderS2, idGridRight);
	entityRelativePositions.setComponent(ContainerSliderS2, component::relativepos(nValueType::percent, ff::vec2f(0.05f, 0.60f), ff::vec2f(0.8f, 0.05f), nPosSide::topLeft));

	//Button SliderS2
	ff::id<entity> SliderS2 = entityManager.addNew();
	entityHierarchy.setParent(SliderS2, ContainerSliderS2);																									//
	entityDrawables.setComponent(SliderS2, component::rect(ff::color::rgb(170, 170, 170, 255)));																	//
	entityRelativePositions.setComponent(SliderS2, component::relativepos(nValueType::px, ff::vec2f(), ff::vec2f(150, 10.0f), nPosSide::topLeft));			//

	//knob SliderH2
	ff::id<entity> SliderKnobS2 = entityManager.addNew();
	entityHierarchy.setParent(SliderKnobS2, SliderS2);																									//
	entityDrawables.setComponent(SliderKnobS2, component::circle(ff::color::rgb(100, 100, 100, 255)));																	//
	entityRelativePositions.setComponent(SliderKnobS2, component::relativepos(nValueType::px, ff::vec2f(5, -5), ff::vec2f(20, 20), nPosSide::topRight));			//

	//text SliderH2
	ff::id<entity> SLiderTextS2 = entityManager.addNew();																													//
	entityHierarchy.setParent(SLiderTextS2, ContainerSliderS2);																												//
	entityRelativePositions.setComponent(SLiderTextS2, component::relativepos(nValueType::px, ff::vec2f(-5, -5), ff::vec2f(), nPosSide::topRight));						//
	entityDrawables.setComponent(SLiderTextS2, component::text("S : 100", 12, ff::color::white()));

	// Container of SliderV2
	ff::id<entity> ContainerSliderV22 = entityManager.addNew();																									//
	entityHierarchy.setParent(ContainerSliderV22, idGridRight);
	entityRelativePositions.setComponent(ContainerSliderV22, component::relativepos(nValueType::percent, ff::vec2f(0.05f, 0.65f), ff::vec2f(0.8f, 0.05f), nPosSide::topLeft));

	//Button SliderV2
	ff::id<entity> SliderV2 = entityManager.addNew();
	entityHierarchy.setParent(SliderV2, ContainerSliderV22);																									//
	entityDrawables.setComponent(SliderV2, component::rect(ff::color::rgb(170, 170, 170, 255)));																	//
	entityRelativePositions.setComponent(SliderV2, component::relativepos(nValueType::px, ff::vec2f(), ff::vec2f(150, 10.0f), nPosSide::topLeft));			//

	//knob SliderV2
	ff::id<entity> SliderKnobV2 = entityManager.addNew();
	entityHierarchy.setParent(SliderKnobV2, SliderV2);																									//
	entityDrawables.setComponent(SliderKnobV2, component::circle(ff::color::rgb(100, 100, 100, 255)));																	//
	entityRelativePositions.setComponent(SliderKnobV2, component::relativepos(nValueType::px, ff::vec2f(5, -5), ff::vec2f(20, 20), nPosSide::topRight));			//

	//text SliderV2
	ff::id<entity> SLiderTextV2 = entityManager.addNew();																													//
	entityHierarchy.setParent(SLiderTextV2, ContainerSliderV22);																												//
	entityRelativePositions.setComponent(SLiderTextV2, component::relativepos(nValueType::px, ff::vec2f(-5, -5), ff::vec2f(), nPosSide::topRight));						//
	entityDrawables.setComponent(SLiderTextV2, component::text("V : 100", 12, ff::color::white()));

	//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//text SliderMinMax
	ff::id<entity> SLiderTextMin4 = entityManager.addNew();																													//
	entityHierarchy.setParent(SLiderTextMin4, idGridRight);																												//
	entityRelativePositions.setComponent(SLiderTextMin4, component::relativepos(nValueType::percent, ff::vec2f(0.05f, 0.75f), ff::vec2f(0.8f, 0.05f), nPosSide::topLeft));
	entityDrawables.setComponent(SLiderTextMin4, component::text("Min Jeuton Jaune", 12, ff::color::white()));

	// Container of SliderH2
	ff::id<entity> ContainerSliderH3 = entityManager.addNew();																									//
	entityHierarchy.setParent(ContainerSliderH3, idGridRight);
	entityRelativePositions.setComponent(ContainerSliderH3, component::relativepos(nValueType::percent, ff::vec2f(0.05f, 0.80f), ff::vec2f(0.8f, 0.05f), nPosSide::topLeft));

	//Button SliderH2
	ff::id<entity> SliderH3 = entityManager.addNew();
	entityHierarchy.setParent(SliderH3, ContainerSliderH3);																									//
	entityDrawables.setComponent(SliderH3, component::rect(ff::color::rgb(170, 170, 170, 255)));																	//
	entityRelativePositions.setComponent(SliderH3, component::relativepos(nValueType::px, ff::vec2f(), ff::vec2f(150, 10.0f), nPosSide::topLeft));			//

	//knob SliderH2
	ff::id<entity> SliderKnobH3 = entityManager.addNew();
	entityHierarchy.setParent(SliderKnobH3, SliderH3);																									//
	entityDrawables.setComponent(SliderKnobH3, component::circle(ff::color::rgb(100, 100, 100, 255)));																	//
	entityRelativePositions.setComponent(SliderKnobH3, component::relativepos(nValueType::px, ff::vec2f(5, -5), ff::vec2f(20, 20), nPosSide::topRight));			//

	//text SliderH2
	ff::id<entity> SLiderTextH3 = entityManager.addNew();																													//
	entityHierarchy.setParent(SLiderTextH3, ContainerSliderH3);																												//
	entityRelativePositions.setComponent(SLiderTextH3, component::relativepos(nValueType::px, ff::vec2f(-5, -5), ff::vec2f(), nPosSide::topRight));						//
	entityDrawables.setComponent(SLiderTextH3, component::text("H : 360", 12, ff::color::white()));

	// Container of SliderS2
	ff::id<entity> ContainerSliderS3 = entityManager.addNew();																									//
	entityHierarchy.setParent(ContainerSliderS3, idGridRight);
	entityRelativePositions.setComponent(ContainerSliderS3, component::relativepos(nValueType::percent, ff::vec2f(0.05f, 0.85f), ff::vec2f(0.8f, 0.05f), nPosSide::topLeft));

	//Button SliderS2
	ff::id<entity> SliderS3 = entityManager.addNew();
	entityHierarchy.setParent(SliderS3, ContainerSliderS3);																									//
	entityDrawables.setComponent(SliderS3, component::rect(ff::color::rgb(170, 170, 170, 255)));																	//
	entityRelativePositions.setComponent(SliderS3, component::relativepos(nValueType::px, ff::vec2f(), ff::vec2f(150, 10.0f), nPosSide::topLeft));			//

	//knob SliderH2
	ff::id<entity> SliderKnobS3 = entityManager.addNew();
	entityHierarchy.setParent(SliderKnobS3, SliderS3);																									//
	entityDrawables.setComponent(SliderKnobS3, component::circle(ff::color::rgb(100, 100, 100, 255)));																	//
	entityRelativePositions.setComponent(SliderKnobS3, component::relativepos(nValueType::px, ff::vec2f(5, -5), ff::vec2f(20, 20), nPosSide::topRight));			//

	//text SliderH2
	ff::id<entity> SLiderTextS3 = entityManager.addNew();																													//
	entityHierarchy.setParent(SLiderTextS3, ContainerSliderS3);																												//
	entityRelativePositions.setComponent(SLiderTextS3, component::relativepos(nValueType::px, ff::vec2f(-5, -5), ff::vec2f(), nPosSide::topRight));						//
	entityDrawables.setComponent(SLiderTextS3, component::text("S : 100", 12, ff::color::white()));

	// Container of SliderV2
	ff::id<entity> ContainerSliderV3 = entityManager.addNew();																									//
	entityHierarchy.setParent(ContainerSliderV3, idGridRight);
	entityRelativePositions.setComponent(ContainerSliderV3, component::relativepos(nValueType::percent, ff::vec2f(0.05f, 0.90f), ff::vec2f(0.8f, 0.05f), nPosSide::topLeft));

	//Button SliderV2
	ff::id<entity> SliderV3 = entityManager.addNew();
	entityHierarchy.setParent(SliderV3, ContainerSliderV3);																									//
	entityDrawables.setComponent(SliderV3, component::rect(ff::color::rgb(170, 170, 170, 255)));																	//
	entityRelativePositions.setComponent(SliderV3, component::relativepos(nValueType::px, ff::vec2f(), ff::vec2f(150, 10.0f), nPosSide::topLeft));			//

	//knob SliderV2
	ff::id<entity> SliderKnobV3 = entityManager.addNew();
	entityHierarchy.setParent(SliderKnobV3, SliderV3);																									//
	entityDrawables.setComponent(SliderKnobV3, component::circle(ff::color::rgb(100, 100, 100, 255)));																	//
	entityRelativePositions.setComponent(SliderKnobV3, component::relativepos(nValueType::px, ff::vec2f(5, -5), ff::vec2f(20, 20), nPosSide::topRight));			//

	//text SliderV2
	ff::id<entity> SLiderTextV3 = entityManager.addNew();																													//
	entityHierarchy.setParent(SLiderTextV3, ContainerSliderV3);																												//
	entityRelativePositions.setComponent(SLiderTextV3, component::relativepos(nValueType::px, ff::vec2f(-5, -5), ff::vec2f(), nPosSide::topRight));						//
	entityDrawables.setComponent(SLiderTextV3, component::text("V : 100", 12, ff::color::white()));

}

void p4ui::updateCameraCalibration(ff::vec2u _windowSize, sf::Sprite _sprite, ff::inputstate _inputState) {

	entityDrawables.get(cameraId) = component::drawable(_sprite); // Update camera sprite

	ff::color col1 = ff::color::rgb(100, 100, 100, 255);// color change for the play button
	if (entityRelativePositions.get(BackButtonId).bounds.contains(_inputState.mousePosition))
	{
		col1 = ff::color::rgb(64, 64, 64, 255);
	}
	entityDrawables.setComponent(BackButtonId, component::rect(col1));

	update(_windowSize, _inputState);
}

void p4ui::updateCameraCalibration(ff::vec2u _windowSize, ff::inputstate _inputState) {

}



void p4ui::deleteUI() {

	entityDrawables.eraseAll();
	entityEventsClick.eraseAll();
	entityRelativePositions.eraseAll();
	entityHierarchy.eraseAll();
}

