#pragma once

#include "VttAudioStreamer/IFade.h"
#include <chrono>

namespace VttAudioStreamer
{

	// Mock implementation of IFade
	class MockFade : public IFade
	{
	public:
		explicit MockFade(std::chrono::milliseconds duration)
			: m_duration(duration)
		{
		}

		std::chrono::milliseconds GetDuration() const override { return m_duration; }

	private:
		std::chrono::milliseconds m_duration;
	};
}