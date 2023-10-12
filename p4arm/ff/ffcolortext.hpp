
#pragma once

#include "ffdynarray.hpp"
#include "ffstring.hpp"
#include "ffunistring.hpp"
#include "ffcolor.hpp"
#include "ffregex.hpp"
#include "ffregex.hpp"

namespace ff
{
	struct colortext
	{
		ff::unistring text;
		dynarray<color> colors;
		dynarray<uint> idxs;

		colortext() {}

		bool hasIntegrityError() const;
		void pushback(ff::unistring _text, ff::color _color = color::rgb(255, 255, 255));
		void pushback(const colortext& _colortext);

		colortext& operator <<(color _color);
		colortext& operator <<(ff::unistring _text);
		colortext& operator <<(const colortext& _text);

		void addColor(ff::color _color, ff::interval<uint> _interval)
		{
			if (_interval.contains(text.size())) { return; }
			
			ff::color currentColor;
			for (uint i = 0; i < idxs.size(); i += 1)
			{
				currentColor = colors[i];

				if (currentColor == _color) { continue; }

				ff::interval<uint> currentInterval = ff::interval<uint>(idxs[i], text.size()); if (i + 1 < idxs.size()) { currentInterval.end = idxs[i + 1]; }

				// overwrite current color fully
				if (currentInterval.isContainedBy(_interval)) { colors[i] = _color; }
				// split current color into 2 sections (and add section in the middle)
				else if (currentInterval.contains(_interval))
				{
					idxs.insert(i + 1, _interval.start); colors.insert(i + 1, _color);
					idxs.insert(i + 2, _interval.end); colors.insert(i + 2, currentColor);
				}
				// shorten current color (make it start later)
				else if (currentInterval.hasStartCutBy(_interval))
				{
					colors[i] = _color;
					idxs.insert(i + 1, _interval.end); colors.insert(i + 1, currentColor);
				}
				// shorten current color (make it end sooner)
				else if (currentInterval.hasEndCutBy(_interval))
				{
					idxs.insert(i + 1, _interval.start); colors.insert(i + 1, _color);
				}
			}

			for (uint i = 0; i < idxs.size();) // cleanup
			{
				if (i + 1 < idxs.size() && idxs[i] == idxs[i + 1]) { idxs.erase(i); colors.erase(i); }
				if (i + 1 < idxs.size() && colors[i] == colors[i + 1]) { idxs.erase(i + 1); colors.erase(i + 1); }
				else { i += 1; }
			}
		}

		void erase(ff::interval<uint> _interval)
		{
			ff::dynarray<ff::interval<uint>> intervals;
			for (uint i = 0; i < idxs.size();)
			{
				ff::interval<uint> interval;
				if (i + 1 < idxs.size()) { interval = ff::interval<uint>(idxs[i], idxs[i + 1]); }
				else { interval = ff::interval<uint>(idxs[i], text.size()); }

				if (interval == _interval) { idxs.erase(i); colors.erase(i); break; }
				else if (interval.isContainedBy(_interval)) { idxs.erase(i); colors.erase(i); }
				else if (interval.hasStartCutBy(_interval)) { idxs[i] = _interval.end; i += 1; }
				else if (interval.hasEndCutBy(_interval)) { i += 1; }
				else if (interval.contains(_interval)) { idxs.insert(i + 1, _interval.end); colors.insert(i + 1, colors[i]); break; }
				else if (!interval.overlaps(_interval)) { i += 1; }
				else { std::cout << "ERROR ERASE COLORTEXT\n"; }
			}
			for (uint i = 0; i < idxs.size(); i += 1) { if (idxs[i] > _interval.start) { idxs[i] -= _interval.size(); } }
			text.erase(_interval);

			for (uint i = 0; i < idxs.size();) // cleanup
			{
				if (i + 1 < idxs.size() && idxs[i] == idxs[i + 1]) { idxs.erase(i); colors.erase(i); }
				else if (i + 1 < idxs.size() && colors[i] == colors[i + 1]) { idxs.erase(i + 1); colors.erase(i + 1); }
				else { i += 1; }
			}

			if (hasIntegrityError()) { std::cout << "ERASE INTEGRITY ERROR\n"; }
		}

		void insert(ff::unistring _string, uint _idx)
		{
			for (uint i = 0; i < idxs.size(); i += 1) { if (idxs[i] > _idx) { idxs[i] += _string.size(); } }
			text.insert(_string, _idx);

			for (uint i = 0; i < idxs.size();) // cleanup
			{
				if (i + 1 < idxs.size() && idxs[i] == idxs[i + 1]) { idxs.erase(i); colors.erase(i); }
				if (i + 1 < idxs.size() && colors[i] == colors[i + 1]) { idxs.erase(i + 1); colors.erase(i + 1); }
				else { i 
					+= 1; }
			}
			if (hasIntegrityError()) { std::cout << "NORMAL INSERT INTEGRITY ERROR\n"; }
		}

		void insert(ff::unistring _string, uint _idx, ff::color _color)
		{
			for (uint i = 0; i < idxs.size(); i += 1)
			{
				ff::interval<uint> interval;
				if (i + 1 < idxs.size()) { interval = ff::interval<uint>(idxs[i], idxs[i + 1]); }
				else { interval = ff::interval<uint>(idxs[i], text.size()); }
				
				if (interval.contains(_idx))
				{
					if (interval.start == _idx) { idxs[i] += _string.size(); idxs.insert(i, _idx); colors.insert(i, _color); i += 1; }
					else { idxs.insert(i + 1, _idx); colors.insert(i + 1, _color); idxs.insert(i + 2, _idx + _string.size()); colors.insert(i + 2, colors[i]); i += 2; }
				}
				else if (interval.end == _idx && i + 1 >= idxs.size()) { idxs.pushback(_idx); colors.pushback(_color); i += 2; }
				else if (idxs[i] > _idx) { idxs[i] += _string.size(); }
			}

			text.insert(_string, _idx);

			for (uint i = 0; i < idxs.size();) // cleanup
			{
				if (i + 1 < idxs.size() && idxs[i] == idxs[i + 1]) { idxs.erase(i); colors.erase(i); }
				if (i + 1 < idxs.size() && colors[i] == colors[i + 1]) { idxs.erase(i + 1); colors.erase(i + 1); }
				else { i += 1; }
			}

			if (hasIntegrityError()) { std::cout << "COLOR INSERT INTEGRITY ERROR\n"; }
		}

		void replace(ff::unistring _match, ff::color _color)
		{
			for (uint i = 0; i < text.size();)
			{
				if (text.findAtIdx(_match, i))
				{
					erase(ff::interval<uint>(i, i + _match.size()));
					addColor(_color, ff::interval<uint>(i, text.size()));
				}
				else { i += 1; }
			}
		}

		void replace(ff::unistring _regex, ff::replacement _replacement)
		{
			ff::dynarray<ff::regex::match> matches = ff::regex::findMatches(text, _regex);
			for (uint i = 0; i < matches.size(); i += 1)
			{
				if (hasIntegrityError()) { std::cout << "REPLACE START INTEGRITY ERROR\n"; }

				erase(matches[i].idxs);
				for (uint j = i + 1; j < matches.size(); j += 1) { matches[j].idxs.start -= matches[i].idxs.size(); matches[j].idxs.end -= matches[i].idxs.size(); }

				unistring replaceStr;
				if (_replacement.strings.size() > 1)
				{
					for (uint j = 0; j < matches[i].groups.size(); j += 1)
					{
						replaceStr += _replacement.strings[j];
						replaceStr += matches[i].groups[j].text;
					}
					replaceStr += _replacement.strings.back();
				}
				else { replaceStr = _replacement.strings[0]; }


				if (hasIntegrityError()) { std::cout << "REPLACE MIDDLE INTEGRITY ERROR\n"; }

				insert(replaceStr, matches[i].idxs.start);
				for (uint j = i + 1; j < matches.size(); j += 1) { matches[j].idxs.start += replaceStr.size(); matches[j].idxs.end += replaceStr.size(); }


				if (hasIntegrityError()) { std::cout << "REPLACE END INTEGRITY ERROR\n"; }
			}
		}

		void replace(ff::unistring _regex, ff::replacement _replacement, ff::color _color)
		{
			ff::dynarray<ff::regex::match> matches = ff::regex::findMatches(text, _regex);
			for (uint i = 0; i < matches.size(); i += 1)
			{
				if (hasIntegrityError()) { std::cout << "REPLACE START COLOR INTEGRITY ERROR\n"; }
				erase(matches[i].idxs);
				for (uint j = i + 1; j < matches.size(); j += 1) { matches[j].idxs.start -= matches[i].idxs.size(); matches[j].idxs.end -= matches[i].idxs.size(); }

				unistring replaceStr;
				if (_replacement.strings.size() > 1)
				{
					for (uint j = 0; j < matches[i].groups.size(); j += 1)
					{
						replaceStr += _replacement.strings[j];
						replaceStr += matches[i].groups[j].text;
					}
					replaceStr += _replacement.strings.back();
				}
				else { replaceStr = _replacement.strings[0]; }
				if (hasIntegrityError()) { std::cout << "REPLACE MIDDLE COLOR INTEGRITY ERROR\n"; }

				insert(replaceStr, matches[i].idxs.start, _color);
				for (uint j = i + 1; j < matches.size(); j += 1) { matches[j].idxs.start += replaceStr.size(); matches[j].idxs.end += replaceStr.size(); }
				if (hasIntegrityError()) { std::cout << "REPLACE END COLOR INTEGRITY ERROR\n"; }
			}
		}

		void paintMatches(ff::string _regex, ff::color _color)
		{
			ff::dynarray<ff::regex::match> matches = ff::regex::findMatches(text, _regex);
			for (uint i = 0; i < matches.size(); i += 1) { addColor(_color, matches[i].idxs); }
		}


		ff::unistring debugText() const
		{
			ff::unistring result = "";
			for (uint i = 0; i < idxs.size(); i += 1)
			{
				if (i + 1 < idxs.size())
				{
					result += text.substring(ff::interval<uint>(idxs[i], idxs[i + 1])) + "\n";
					result += "\t- [" + (ff::string)idxs[i] + "," + (ff::string)idxs[i + 1] + "[   " + (ff::string)colors[i].r + "," + (ff::string)colors[i].g + "," + (ff::string)colors[i].b + "\n";
				}
				else
				{
					result += text.substring(ff::interval<uint>(idxs[i], text.size())) + "\n";
					result += "\t- [" + (ff::string)idxs[i] + "," + (ff::string)text.size() + "[   " + (ff::string)colors[i].r + "," + (ff::string)colors[i].g + "," + (ff::string)colors[i].b + "\n";
				}
				result += "\n";
			}
			return result;
		}

		ff::colortext debugTextColor()
		{
			ff::colortext result;
			for (uint i = 0; i < idxs.size(); i += 1)
			{
				if (i + 1 < idxs.size())
				{
					result << colors[i] << text.substring(ff::interval<uint>(idxs[i], idxs[i + 1])) + "\n";
					result << ff::color::white() << "\t- [" + (ff::string)idxs[i] + "," + (ff::string)idxs[i + 1] + "[   " + (ff::string)colors[i].r + "," + (ff::string)colors[i].g + "," + (ff::string)colors[i].b + "\n";
				}
				else
				{
					result << colors[i] << text.substring(ff::interval<uint>(idxs[i], text.size())) + "\n";
					result << ff::color::white() << "\t- [" + (ff::string)idxs[i] + "," + (ff::string)text.size() + "[   " + (ff::string)colors[i].r + "," + (ff::string)colors[i].g + "," + (ff::string)colors[i].b + "\n";
				}
				result << (ff::string)"\n";
			}
			return result;
		}

		colortext& indent(uint _amount);
	};
}







bool ff::colortext::hasIntegrityError() const
{
	for (uint i = 0; i < idxs.size(); i += 1)
	{
		if (idxs[i] >= text.size())
		{
			std::cout << "IDX ERROR (" << idxs[i] << "/" << text.size() << ")\n";

			ff::unistring x = text.substring(ff::interval<uint>(idxs[i - 1], text.size()));
			for (uint j = 0; j < x.size(); j += 1) { std::cout << (char)x[j]; }
			std::cout << "\n";
			return true;
		}
	}
	return false;
}
void ff::colortext::pushback(ff::unistring _text, ff::color _color)
{
	idxs.pushback(text.size());
	text += _text;
	colors.pushback(_color);
}
void ff::colortext::pushback(const colortext& _colortext)
{
	for (uint i = 0; i < _colortext.colors.size(); i += 1)
	{
		if (idxs.size() > 0 && _colortext.idxs[i] + text.size() == idxs.back()) { colors.back() = _colortext.colors[i]; }
		else
		{
			colors.pushback(_colortext.colors[i]);
			idxs.pushback(_colortext.idxs[i] + text.size());
		}
	}
	text += _colortext.text;
}
ff::colortext& ff::colortext::operator <<(color _color)
{
	if (idxs.size() > 0 && idxs.back() == text.size()) { colors.back() = _color; return *this; }
	else { colors.pushback(_color); idxs.pushback(text.size()); return *this; }
}
ff::colortext& ff::colortext::operator <<(ff::unistring _text)
{
	if (_text.size() == 0) { return *this; }
	if (idxs.size() == 0) { idxs.pushback(0); colors.pushback(color::rgb(255, 255, 255)); }
	text += _text;
	return *this;
}
ff::colortext& ff::colortext::operator <<(const colortext& _text) { pushback(_text); return *this; }
ff::colortext& ff::colortext::indent(uint _amount)
{
	ff::unistring indents;
	for (uint i = 0; i < _amount; i += 1) { indents += "\t"; }
	this->insert(indents, 0);

	for (uint i = 0; i < text.size(); i += 1) { if (text[i] == '\n') { this->insert(indents, i + 1); } }
	return *this;
}