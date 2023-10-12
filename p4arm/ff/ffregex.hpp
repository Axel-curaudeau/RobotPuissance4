
#pragma once

#include "ffdynarray.hpp"
#include "ffinterval.hpp"
#include "ffunistring.hpp"
#include <regex>
#include <string>
#include <iostream>

namespace ff { struct unistring; }
namespace ff
{
	namespace regex
	{
		struct group;
		struct match;
		ff::dynarray<match> findMatches(ff::unistring _string, ff::unistring _regex);
	}
}

namespace ff
{
	namespace regex
	{
		struct group
		{
			ff::unistring text;
			ff::interval<uint> idxs;
		};
		struct match
		{
			ff::unistring text;
			ff::interval<uint> idxs;
			ff::dynarray<group> groups;
		};

		ff::dynarray<match> findMatches(ff::unistring _string, ff::unistring _regex)
		{
			ff::dynarray<match> result;

			std::smatch matches;
			std::regex regex = std::regex(std::string(_regex));
			std::string str = _string;

			for (std::sregex_iterator it = std::sregex_iterator(str.begin(), str.end(), regex); it != std::sregex_iterator(); it++)
			{
				uint startIdx = (uint)it->position();
				uint endIdx = (uint)it->position() + (uint)it->length();

				match currentMatch;
				currentMatch.text = it->str();
				currentMatch.idxs.start = (uint)it->position();
				currentMatch.idxs.end = (uint)it->position() + (uint)it->length();

				for (uint i = 1; i < it->size(); i += 1)
				{
					group iGroup;
					iGroup.text = it->str(i);
					iGroup.idxs.start = (uint)it->position(i);
					iGroup.idxs.end = (uint)it->position(i) + (uint)it->length(i);

					currentMatch.groups.pushback(iGroup);
				}

				result.pushback(currentMatch);
			}

			return result;
		}
	}
}