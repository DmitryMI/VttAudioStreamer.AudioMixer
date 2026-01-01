#pragma once

#include "VttAudioStreamer/IPcmFrame.h"
#include <cstdint>

namespace VttAudioStreamer
{
	class PcmFrame : public IPcmFrame
	{
	public:
		PcmFrame(const std::vector<float>& samples, uint32_t sampleRate, uint16_t channelCount)
			: m_Samples(samples), m_SampleRate(sampleRate), m_ChannelCount(channelCount)
		{
		}

		virtual ~PcmFrame() = default;

		std::vector<float> GetSamples() const override
		{
			return m_Samples;
		}

		uint32_t GetSampleRate() const override
		{
			return m_SampleRate;
		}

		uint32_t GetChannelCount() const override
		{
			return m_ChannelCount;
		}

		std::optional<std::chrono::milliseconds> GetTimestamp() const override
		{
			return std::chrono::milliseconds(0);
		}

	private:
		std::vector<float> m_Samples;
		uint32_t m_SampleRate;
		uint16_t m_ChannelCount;
	};
}