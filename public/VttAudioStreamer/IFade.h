#pragma once

#include <chrono>

namespace VttAudioStreamer
{
	class IFade
	{
	public:
		virtual ~IFade() = default;

		virtual std::chrono::milliseconds GetDuration() const = 0;
	};
}