#pragma once

#include "VttAudioStreamer/IPlaybackRange.h"
#include <chrono>

namespace VttAudioStreamer
{

	// Mock implementation of IPlaybackRange
	class PlaybackRangeMock : public IPlaybackRange
	{
	public:
		PlaybackRangeMock(std::chrono::milliseconds start, std::chrono::milliseconds end)
			: m_start(start), m_end(end)
		{
		}

		std::chrono::milliseconds GetStart() const override { return m_start; }
		std::chrono::milliseconds GetEnd() const override { return m_end; }

	private:
		std::chrono::milliseconds m_start;
		std::chrono::milliseconds m_end;
	};
}