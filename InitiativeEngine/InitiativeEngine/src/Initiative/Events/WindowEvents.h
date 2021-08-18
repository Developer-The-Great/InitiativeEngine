#pragma once
#include "Event.h"

namespace itv
{
	class WindowResizeEvent final : public EventBase
	{
	public:
		WindowResizeEvent(int pWidth, int pBreadth) : width(pWidth), breadth(pBreadth) {}
		~WindowResizeEvent() = default;

		std::tuple<int, int> GetExtents() const { return std::make_tuple(width, breadth); }


	private:

		int width, breadth;

	};

	class WindowCloseEvent final : public EventBase
	{
	public:
		WindowCloseEvent() = default;
		~WindowCloseEvent() = default;

	};


}

