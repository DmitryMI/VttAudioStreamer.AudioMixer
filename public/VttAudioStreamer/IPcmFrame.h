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
		virtual uint32_t GetSampleRate() const = 0;
		virtual uint32_t GetChannelCount() const = 0;
		virtual std::optional<std::chrono::milliseconds> GetTimestamp() const = 0;
	};
}