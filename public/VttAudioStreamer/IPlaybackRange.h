#pragma once

#include <chrono>

namespace VttAudioStreamer
{
	class IPlaybackRange
	{
	public:
		virtual ~IPlaybackRange() = default;
		virtual std::chrono::milliseconds GetStart() const = 0;
		virtual std::chrono::milliseconds GetEnd() const = 0;
	};
}