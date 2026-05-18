#pragma once

#include <vector>
#include <chrono>
#include "VttAudioStreamer/IPcmConfig.h"

namespace VttAudioStreamer
{
	class IPcmFrame
	{
	public:
		virtual ~IPcmFrame() = default;
		virtual std::vector<float> GetSamples() const = 0;
		virtual std::optional<std::chrono::milliseconds> GetTimestamp() const = 0;
		virtual std::shared_ptr<IPcmConfig> GetPcmConfig() const = 0;
	};
}