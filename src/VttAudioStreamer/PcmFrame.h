#pragma once

#include "VttAudioStreamer/IPcmFrame.h"
#include <cstdint>

namespace VttAudioStreamer
{
	class PcmConfig : public IPcmConfig
	{
	public:
		PcmConfig(uint32_t sampleRate, uint16_t channelCount)
			: m_SampleRate(sampleRate), m_ChannelCount(channelCount)
		{
		}
		virtual ~PcmConfig() = default;
		uint32_t GetSampleRate() const override
		{
			return m_SampleRate;
		}
		uint32_t GetChannelCount() const override
		{
			return m_ChannelCount;
		}
	private:
		uint32_t m_SampleRate;
		uint32_t m_ChannelCount;
	};

	class PcmFrame : public IPcmFrame
	{
	public:
		PcmFrame(const std::vector<float>& samples, uint32_t sampleRate, uint16_t channelCount)
			: m_Samples(samples), m_PcmConfig(std::make_shared<PcmConfig>(m_SampleRate, m_ChannelCount))
		{
		}

		PcmFrame(const std::vector<float>& samples, std::shared_ptr<IPcmConfig> config)
			: m_Samples(samples), m_PcmConfig(config)
		{
		}

		PcmFrame(const std::vector<float>& samples, uint32_t sampleRate, uint16_t channelCount, std::chrono::milliseconds timestamp)
			: m_Samples(samples), m_SampleRate(sampleRate), m_ChannelCount(channelCount), m_Timestamp(timestamp)
		{
		}

		virtual ~PcmFrame() = default;

		std::vector<float> GetSamples() const override
		{
			return m_Samples;
		}

		std::optional<std::chrono::milliseconds> GetTimestamp() const override
		{
			return m_Timestamp;
		}

	private:
		std::vector<float> m_Samples;
		uint32_t m_SampleRate;
		uint32_t m_ChannelCount;
		std::shared_ptr<IPcmConfig> m_PcmConfig;
		std::optional<std::chrono::milliseconds> m_Timestamp;
	};
}