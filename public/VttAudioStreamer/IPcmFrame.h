#pragma once

#include <vector>
#include <chrono>

namespace VttAudioStreamer
{
	class IPcmFrame
	{
	public:
		virtual ~IPcmFrame() = default;
		virtual std::vector<float> GetSamples() const = 0;
		virtual int GetSampleRate() const = 0;
		virtual int GetChannelCount() const = 0;
		virtual std::chrono::milliseconds GetTimestamp() const = 0;
	};
}