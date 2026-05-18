#pragma once

namespace VttAudioStreamer
{
	class IPcmConfig
	{
	public:
		virtual ~IPcmConfig() = default;
		virtual uint32_t GetSampleRate() const = 0;
		virtual uint32_t GetChannelCount() const = 0;
	};
}