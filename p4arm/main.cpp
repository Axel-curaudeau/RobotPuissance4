


// INCLUDE LIB: SFML (IMPORTANT: Additionally, project settings MUST include "./" (the directory where the .sln file is) as an include directory because C++ compilers didn't let me add it with comments)
#if defined(_DEBUG)
	// build debug
	#if defined(_WIN64)
		// build x64
		#pragma comment(lib, "lib/sfml/x64_debug/sfml-window-d.lib")
		#pragma comment(lib, "lib/sfml/x64_debug/sfml-graphics-d.lib")
		#pragma comment(lib, "lib/sfml/x64_debug/sfml-system-d.lib")
	#else
		// build x32
		#pragma comment(lib, "lib/sfml/x86_debug/sfml-window-d.lib")
		#pragma comment(lib, "lib/sfml/x86_debug/sfml-graphics-d.lib")
		#pragma comment(lib, "lib/sfml/x86_debug/sfml-system-d.lib")
	#endif
#else
	// build release
	#if defined(_WIN64)
		// build x64
		#pragma comment(lib, "lib/sfml/x64_release/sfml-window.lib")
		#pragma comment(lib, "lib/sfml/x64_release/sfml-graphics.lib")
		#pragma comment(lib, "lib/sfml/x64_release/sfml-system.lib")
	#else
		// build x32
		#pragma comment(lib, "lib/sfml/x86_release/sfml-window.lib")
		#pragma comment(lib, "lib/sfml/x86_release/sfml-graphics.lib")
		#pragma comment(lib, "lib/sfml/x86_release/sfml-system.lib")
	#endif
#endif




// INCLUDE LIB: DOBOT
#if defined(_DEBUG)
	// build debug
	#if defined(_WIN64)
		// build x64
		#error No Dobot library for debug x64
	#else
		// build x32
		#pragma comment(lib, "lib/dobot/x86_release/DobotDll.lib") // uses release version anyway
	#endif
#else
	// build release
	#if defined(_WIN64)
		// build x64
		#error No Dobot library for release x64
	#else
		// build x32
		#pragma comment(lib, "lib/dobot/x86_release/DobotDll.lib")
	#endif
#endif




// INCLUDE LIB: OPENCV
#if defined(_DEBUG)
	// build debug
	#if defined(_WIN64)
		// build x64
		#error No OpenCV library for debug x64
	#else
		// build x32
		#pragma comment(lib, "lib/opencv/x86_debug/opencv_calib3d455d.lib")
		#pragma comment(lib, "lib/opencv/x86_debug/opencv_core455d.lib")
		#pragma comment(lib, "lib/opencv/x86_debug/opencv_dnn455d.lib")
		#pragma comment(lib, "lib/opencv/x86_debug/opencv_features2d455d.lib")
		#pragma comment(lib, "lib/opencv/x86_debug/opencv_flann455d.lib")
		#pragma comment(lib, "lib/opencv/x86_debug/opencv_gapi455d.lib")
		#pragma comment(lib, "lib/opencv/x86_debug/opencv_highgui455d.lib")
		#pragma comment(lib, "lib/opencv/x86_debug/opencv_imgcodecs455d.lib")
		#pragma comment(lib, "lib/opencv/x86_debug/opencv_imgproc455d.lib")
		#pragma comment(lib, "lib/opencv/x86_debug/opencv_ml455d.lib")
		#pragma comment(lib, "lib/opencv/x86_debug/opencv_objdetect455d.lib")
		#pragma comment(lib, "lib/opencv/x86_debug/opencv_photo455d.lib")
		#pragma comment(lib, "lib/opencv/x86_debug/opencv_stitching455d.lib")
		#pragma comment(lib, "lib/opencv/x86_debug/opencv_ts455d.lib")
		#pragma comment(lib, "lib/opencv/x86_debug/opencv_video455d.lib")
		#pragma comment(lib, "lib/opencv/x86_debug/opencv_videoio455d.lib")
	#endif
#else
	// build release
	#if defined(_WIN64)
		// build x64
		#error No OpenCV library for release x64
	#else
		// build x32
		#pragma comment(lib, "lib/opencv/x86_release/opencv_calib3d455.lib")
		#pragma comment(lib, "lib/opencv/x86_release/opencv_core455.lib")
		#pragma comment(lib, "lib/opencv/x86_release/opencv_dnn455.lib")
		#pragma comment(lib, "lib/opencv/x86_release/opencv_features2d455.lib")
		#pragma comment(lib, "lib/opencv/x86_release/opencv_flann455.lib")
		#pragma comment(lib, "lib/opencv/x86_release/opencv_gapi455.lib")
		#pragma comment(lib, "lib/opencv/x86_release/opencv_highgui455.lib")
		#pragma comment(lib, "lib/opencv/x86_release/opencv_imgcodecs455.lib")
		#pragma comment(lib, "lib/opencv/x86_release/opencv_imgproc455.lib")
		#pragma comment(lib, "lib/opencv/x86_release/opencv_ml455.lib")
		#pragma comment(lib, "lib/opencv/x86_release/opencv_objdetect455.lib")
		#pragma comment(lib, "lib/opencv/x86_release/opencv_photo455.lib")
		#pragma comment(lib, "lib/opencv/x86_release/opencv_stitching455.lib")
		#pragma comment(lib, "lib/opencv/x86_release/opencv_ts455.lib")
		#pragma comment(lib, "lib/opencv/x86_release/opencv_video455.lib")
		#pragma comment(lib, "lib/opencv/x86_release/opencv_videoio455.lib")
	#endif
#endif



#include <SFML/Window.hpp>
#include "p4states.hpp"

#include "p4states.hpp"




int main()
{
	static dobot bot;
	sf::RenderWindow& window = p4ui::window;
	window.create(sf::VideoMode(800, 600), "Connect 4 robot");


	ff::inputstate inputState;

	ff::timer ticks;

	bot.config.load();


	p4ui::initMainMenu();
	p4ui::updateMainMenu(ff::vec2u(window.getSize().x, window.getSize().y));

	while (window.isOpen())
	{
		if (!ticks.tickEveryMilli(1000 / 60)) { ff::sleep(1); continue; }

		inputState.applyPending();	   // (<- applies pending inputs from previous iteration)
		inputState.pollEvents(window); // Update the window inputs

		bot.connect(); // Attempt to connect to dobot
		bot.ping();	   // (<- verifies that dobot is still connected)

		//After the HOME, moove dobot to a better position before starting
		bot.moveTo(bot.getHighAmmoPos(0, 0));

		if (p4ui::uiState == p4ui::UIState::P4) {


			cvimage debugImg;
			
			p4::states::uiExchange exchange;

			exchange.board = p4ui::board;											// (<- copy ui state)
			exchange.ammoState = p4ui::ammoState;									// (<- copy ui state)
			exchange.editMode = p4ui::editMode;										// (<- copy ui state)
			p4::states::nState state = p4::states::tick(bot, debugImg, exchange);	// Tick the dobot state machine
			p4ui::board = exchange.board;											// (<- apply modified ui state)
			p4ui::ammoState = exchange.ammoState;									// (<- apply modified ui state)
			p4ui::editMode = exchange.editMode;										// (<- apply modified ui state)


			static sf::Texture texture;							 // (<- texture is static to improve performance)
			sf::Sprite sprite = debugImg.drawToTexture(texture); // Transform the debug image to a sprite
			
			try {
				p4ui::updateP4(ff::vec2u(window.getSize().x, window.getSize().y), sprite, bot.isConnected(), state == p4::states::nState::waitingForPlayer, inputState);

				p4ui::updateP4(ff::vec2u(window.getSize().x, window.getSize().y), inputState);
			}
			catch (std::out_of_range exception) {
				std::cout << "Ereur out of range !!!" << std::endl;
			}
		}
		else if (p4ui::uiState == p4ui::UIState::MainMenu) {
			p4ui::updateMainMenu(ff::vec2u(window.getSize().x, window.getSize().y), inputState);
		}
		else if (p4ui::uiState == p4ui::UIState::Settings) {
			p4ui::updateSettings(ff::vec2u(window.getSize().x, window.getSize().y), inputState);
		}
		else if (p4ui::uiState == p4ui::UIState::CalibrationCam) {
			
			bot.connect(); // Attempt to connect to dobot
			bot.ping();	   // (<- verifies that dobot is still connected)

			cvimage debugImg;

			p4::states::uiExchange exchange;

			exchange.board = p4ui::board;											// (<- copy ui state)
			exchange.ammoState = p4ui::ammoState;									// (<- copy ui state)
			exchange.editMode = p4ui::editMode;										// (<- copy ui state)
			p4::states::nState state = p4::states::tick(bot, debugImg, exchange);	// Tick the dobot state machine
			p4ui::board = exchange.board;											// (<- apply modified ui state)
			p4ui::ammoState = exchange.ammoState;									// (<- apply modified ui state)
			p4ui::editMode = exchange.editMode;										// (<- apply modified ui state)


			static sf::Texture texture;							 // (<- texture is static to improve performance)
			sf::Sprite sprite = debugImg.drawToTexture(texture); // Transform the debug image to a sprite
			
			p4ui::updateCameraCalibration(ff::vec2u(window.getSize().x, window.getSize().y),sprite,inputState);
		}
		else if (p4ui::uiState == p4ui::UIState::CalibrationRobot) {
			p4ui::updateRobotCalibration(ff::vec2u(window.getSize().x, window.getSize().y), inputState);
		}

		window.clear();		  //
		p4ui::draw(window);	  // 
		window.display();	  // "Clear, draw, display" section to display the new image to the window
	}


	return 0;
}



/*

int main()
{
	sf::RenderWindow window;
	window.create(sf::VideoMode(800, 600), "Connect 4 robot");


	ff::inputstate inputState;

	ff::timer ticks;

	dobot bot;
	//bot.config.load();


	p4ui::initialize();
	p4ui::update(ff::vec2u(window.getSize().x, window.getSize().y));

	while (window.isOpen())
	{
		if (!ticks.tickEveryMilli(1000 / 60)) { ff::sleep(1); continue; }

		inputState.applyPending();	   // (<- applies pending inputs from previous iteration)
		inputState.pollEvents(window); // Update the window inputs

		if (p4ui::uiState == p4ui::UIState::P4) {


			bot.connect(); // Attempt to connect to dobot
			bot.ping();	   // (<- verifies that dobot is still connected)

			cvimage debugImg;

			p4::states::uiExchange exchange;

			exchange.board = p4ui::board;											// (<- copy ui state)
			exchange.ammoState = p4ui::ammoState;									// (<- copy ui state)
			exchange.editMode = p4ui::editMode;										// (<- copy ui state)
			p4::states::nState state = p4::states::tick(bot, debugImg, exchange);	// Tick the dobot state machine
			p4ui::board = exchange.board;											// (<- apply modified ui state)
			p4ui::ammoState = exchange.ammoState;									// (<- apply modified ui state)
			p4ui::editMode = exchange.editMode;										// (<- apply modified ui state)


			static sf::Texture texture;							 // (<- texture is static to improve performance)
			sf::Sprite sprite = debugImg.drawToTexture(texture); // Transform the debug image to a sprite


			p4ui::update(ff::vec2u(window.getSize().x, window.getSize().y), sprite, bot.isConnected(), state == p4::states::nState::waitingForPlayer, inputState);
		}
		else if (p4ui::uiState == p4ui::UIState::MainMenu) {
			p4ui::updateMainMenu(ff::vec2u(window.getSize().x, window.getSize().y), inputState);
		}

		window.clear();		  //
		p4ui::draw(window);	  // 
		window.display();	  // "Clear, draw, display" section to display the new image to the window
	}


	return 0;
}

*/