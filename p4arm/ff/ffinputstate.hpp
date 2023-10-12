
#pragma once

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include "ffvec2.hpp"
#include "ffdynarray.hpp"

#include "ffinputevent.hpp"


namespace ff
{
	namespace mouseButtonState { enum type { pressed = 0, released }; }
	namespace mouseButtonLabel { enum type { leftButton = 0, rightButton, scrollButton }; }

	struct eventClickRelease { ff::vec2i clickPos; ff::vec2i releasePos; };
	struct eventDrag { ff::vec2i heldStartPos; ff::vec2i heldEndPos; };

	struct inputstate
	{
		mouseButtonState::type mouseLeftButton;
		ff::vec2i mousePosition;
		ff::vec2i lastClickPosition;

		ff::dynarray<ff::inputevent> pendingEvents;

		void pollEvents(sf::RenderWindow& _window);

		ff::dynarray<eventClickRelease> getClickReleaseEvents();
		ff::dynarray<eventDrag> getDragEvents();
		int getTotalScroll();
		void applyPending();
	};
}






void ff::inputstate::pollEvents(sf::RenderWindow& _window)
{
	sf::Event ev;
	while (_window.pollEvent(ev))
	{
		if (ev.type == sf::Event::Closed) { _window.close(); }
		else if (ev.type == sf::Event::Resized) { _window.setView(sf::View(sf::FloatRect(0, 0, (float)_window.getSize().x, (float)_window.getSize().y))); }
		else if (ev.type == sf::Event::MouseButtonPressed) { pendingEvents.pushback(ff::inputevent::mousePress(ff::vec2i(ev.mouseButton.x, ev.mouseButton.y), ev.mouseButton.button)); }
		else if (ev.type == sf::Event::MouseButtonReleased) { pendingEvents.pushback(ff::inputevent::mouseRelease(ff::vec2i(ev.mouseButton.x, ev.mouseButton.y), ev.mouseButton.button)); }
		else if (ev.type == sf::Event::MouseMoved) { pendingEvents.pushback(ff::inputevent::mouseMove(ff::vec2i(ev.mouseMove.x, ev.mouseMove.y))); }
		else if (ev.type == sf::Event::MouseWheelScrolled) { pendingEvents.pushback(ff::inputevent::mouseWheel((int)ev.mouseWheelScroll.delta)); }
	}
}
ff::dynarray<ff::eventClickRelease> ff::inputstate::getClickReleaseEvents()
{
	ff::dynarray<eventClickRelease> result;

	bool held = false;
	eventClickRelease temp;
	if (mouseLeftButton == mouseButtonState::pressed) { temp.clickPos = lastClickPosition; held = true; }

	for (uint i = 0; i < pendingEvents.size(); i += 1)
	{
		if (pendingEvents[i].type == evnt::mousePress) { held = true; temp.clickPos = pendingEvents[i].pos; }
		else if (held && pendingEvents[i].type == evnt::mouseRelease) { held = false; temp.releasePos = pendingEvents[i].pos; result.pushback(temp); }
	}
	return result;
}
ff::dynarray<ff::eventDrag> ff::inputstate::getDragEvents()
{
	ff::dynarray<eventDrag> result;

	bool held = false;
	eventDrag temp;
	if (mouseLeftButton == mouseButtonState::pressed) { temp.heldStartPos = mousePosition; held = true; }

	for (uint i = 0; i < pendingEvents.size(); i += 1)
	{
		if (pendingEvents[i].type == evnt::mousePress) { held = true; temp.heldStartPos = pendingEvents[i].pos; }
		else if (held && pendingEvents[i].type == evnt::mouseRelease) { held = false; temp.heldEndPos = pendingEvents[i].pos; result.pushback(temp); }
		else if (held && pendingEvents[i].type == evnt::mouseMove) { temp.heldEndPos = pendingEvents[i].pos; result.pushback(temp); temp.heldStartPos = pendingEvents[i].pos; }
	}
	return result;
}
int ff::inputstate::getTotalScroll()
{
	int totalScroll = 0;
	for (uint i = 0; i < pendingEvents.size(); i += 1)
	{
		if (pendingEvents[i].type == evnt::mouseWheel) { totalScroll += pendingEvents[i].scroll; }
	}
	return totalScroll;
}
void ff::inputstate::applyPending()
{
	for (uint i = 0; i < pendingEvents.size(); i += 1)
	{
		if (pendingEvents[i].type == evnt::mousePress) { mousePosition = pendingEvents[i].pos; lastClickPosition = pendingEvents[i].pos; mouseLeftButton = mouseButtonState::pressed; }
		else if (pendingEvents[i].type == evnt::mouseRelease) { mousePosition = pendingEvents[i].pos; mouseLeftButton = mouseButtonState::released; }
		else if (pendingEvents[i].type == evnt::mouseMove) { mousePosition = pendingEvents[i].pos; }
	}
	pendingEvents.clear();
}

