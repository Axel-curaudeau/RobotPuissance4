
#pragma once

#include "ffcompcontainer.hpp"
#include "ffcomphierarchy.hpp"
#include "ffcompid.hpp"

namespace ff
{
	namespace dashlog
	{
		namespace nSend { enum type : byte { text = 1, containerSwitch = 2, containerHierarchy = 3, colorSwitch = 4 }; }

		struct container {};

		struct textSend
		{
			ff::string content;
			uint serialLoad(ff::rawmem _mem) {  }
		};
		struct containerSwitchSend
		{
			ff::id<container> target;
			containerSwitchSend(ff::id<container> _target) { target = _target; }
		};
		struct containerHierarchySend { ff::id<container> parent; ff::id<container> child; containerHierarchySend(ff::id<container> _parent, ff::id<container> _child) { parent = _parent; child = _child; } };
		struct colorSwitchSend { ff::color color; };
	}
}



