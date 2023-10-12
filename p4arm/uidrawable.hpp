
#pragma once


#include <SFML/Graphics.hpp>


#include "ff/ffunistring.hpp"
#include "ff/ffrect.hpp"
#include "ff/ffcolor.hpp"


enum class nDrawableType { text, rect, circle, sprite, slider };
namespace component
{
	struct text
	{
		ff::unistring txt; uint size = 10; ff::color color; sf::Text sfmlText;
		text() {}
		text(ff::unistring _txt, uint _size, ff::color _color) { txt = _txt; size = _size; color = _color; }
	};
	struct rect
	{
		ff::color color; sf::RectangleShape sfmlRect;
		rect() {}
		rect(ff::color _color) { color = _color; }
	};
	struct circle
	{
		ff::color color; sf::CircleShape sfmlCircle;
		circle() {}
		circle(ff::color _color) { color = _color; }
	};
	struct sprite
	{
		sf::Sprite sfmlSprite;
		sprite() {}
		sprite(sf::Sprite _sprite) { sfmlSprite = _sprite; }
	};
	struct slider
	{
		ff::color color; sf::Text sfmlText; int value; int maxValue;
		slider(){}
		slider(ff::color _color, int _maxValue) { maxValue = _maxValue; color = _color; }
	};


	struct drawable
	{
		nDrawableType type;

		component::text text;
		component::rect rect;
		component::circle circle;
		component::sprite sprite;
		component::slider slider;

		drawable();
		drawable(component::text _text);
		drawable(component::rect _rect);
		drawable(component::circle _circle);
		drawable(component::sprite _sprite);
		void update(ff::rect<int> _bounds, const sf::Font& _defaultFont);
		void draw(sf::RenderWindow& _window);
	};
}







component::drawable::drawable() {}
component::drawable::drawable(component::text _text) { type = nDrawableType::text; text = _text; }
component::drawable::drawable(component::rect _rect) { type = nDrawableType::rect; rect = _rect; }
component::drawable::drawable(component::circle _circle) { type = nDrawableType::circle; circle = _circle; }
component::drawable::drawable(component::sprite _sprite) { type = nDrawableType::sprite; sprite = _sprite; }
void component::drawable::update(ff::rect<int> _bounds, const sf::Font& _defaultFont)
{
	if (type == nDrawableType::text)
	{
		text.sfmlText.setString((std::string)text.txt);
		text.sfmlText.setPosition((float)_bounds.left, (float)_bounds.top);
		text.sfmlText.setCharacterSize(text.size);
		text.sfmlText.setFillColor(sf::Color(text.color.r, text.color.g, text.color.b, text.color.opacity));
		text.sfmlText.setFont(_defaultFont);
	}
	else if (type == nDrawableType::rect)
	{
		rect.sfmlRect.setPosition((float)_bounds.left, (float)_bounds.top);
		rect.sfmlRect.setSize(sf::Vector2f((float)_bounds.length(), (float)_bounds.height()));
		rect.sfmlRect.setFillColor(sf::Color(rect.color.r, rect.color.g, rect.color.b, rect.color.opacity));
	}
	else if (type == nDrawableType::circle)
	{
		circle.sfmlCircle.setPosition((float)_bounds.left, (float)_bounds.top);
		circle.sfmlCircle.setRadius(_bounds.length() < _bounds.height() ? ((float)_bounds.length() / 2) : ((float)_bounds.height() / 2));
		circle.sfmlCircle.setFillColor(sf::Color(circle.color.r, circle.color.g, circle.color.b, circle.color.opacity));
	}
	else if (type == nDrawableType::sprite)
	{
		sprite.sfmlSprite.setPosition((float)_bounds.left, (float)_bounds.top);
		ff::vec2f size = ff::vec2f(sprite.sfmlSprite.getLocalBounds().width, sprite.sfmlSprite.getLocalBounds().height);
		if (size.x == 0.0f || size.y == 0.0f) { return; }
		sprite.sfmlSprite.setScale(_bounds.length() / size.x, _bounds.height() / size.y);
	}
	else if (type == nDrawableType::slider)
	{
		sprite.sfmlSprite.setPosition((float)_bounds.left, (float)_bounds.top);
		ff::vec2f size = ff::vec2f(sprite.sfmlSprite.getLocalBounds().width, sprite.sfmlSprite.getLocalBounds().height);
		if (size.x == 0.0f || size.y == 0.0f) { return; }
		sprite.sfmlSprite.setScale(_bounds.length() / size.x, _bounds.height() / size.y);
	}
}
void component::drawable::draw(sf::RenderWindow& _window)
{
	if (type == nDrawableType::text) { _window.draw(text.sfmlText); }
	else if (type == nDrawableType::rect) { _window.draw(rect.sfmlRect); }
	else if (type == nDrawableType::circle) { _window.draw(circle.sfmlCircle); }
	else if (type == nDrawableType::sprite) { _window.draw(sprite.sfmlSprite); }
	//else if (type == nDrawableType::slider) { _window.draw(slider); }
}
