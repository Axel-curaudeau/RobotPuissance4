
#pragma once

#include "ffsetup.hpp"
#include "ffvec2.hpp"

namespace ff
{
	namespace evnt { enum type { mousePress, mouseRelease, mouseMove, mouseWheel }; }

	struct inputevent
	{
		evnt::type type;
		ff::vec2i pos;
		uint button = -1;
		int scroll = -9999;

		static inputevent mousePress(ff::vec2i _pos, uint _button) { inputevent result; result.type = evnt::mousePress; result.pos = _pos; result.button = _button; return result; }
		static inputevent mouseRelease(ff::vec2i _pos, uint _button) { inputevent result; result.type = evnt::mouseRelease; result.pos = _pos; result.button = _button; return result; }
		static inputevent mouseMove(ff::vec2i _pos) { inputevent result; result.type = evnt::mouseMove; result.pos = _pos; return result; }
		static inputevent mouseWheel(int _scroll) { inputevent result; result.type = evnt::mouseWheel; result.scroll = _scroll; return result; }
	};
}