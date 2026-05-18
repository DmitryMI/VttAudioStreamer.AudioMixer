#pragma once 

#include "VttAudioStreamer/IAudioSample.h"

namespace VttAudioStreamer
{
	// Mock implementation of IAudioSample
	class AudioSampleMock : public IAudioSample
	{
	public:
		AudioSampleMock(
			const std::string& name,
			float frequency,
			uint32_t sampleRate,
			uint32_t channelCount,
			std::chrono::milliseconds duration)
			: m_name(name)
			, m_frequency(frequency)
			, m_sampleRate(sampleRate)
			, m_channelCount(channelCount)
			, m_duration(duration)
		{
		}

		std::string GetName() const override { return m_name; }

		std::shared_ptr<IPlaybackRange> GetPlaybackRange() const override
		{
			return std::make_shared<PlaybackRangeMock>(
				std::chrono::milliseconds(0),
				m_duration);
		}

		std::shared_ptr<IAsyncIterablePcmFrame> GetPcmStream() const override
		{
			return std::make_shared<SineWaveGenerator>(
				m_frequency,
				m_sampleRate,
				m_channelCount,
				m_duration);
		}

	private:
		std::string m_name;
		float m_frequency;
		uint32_t m_sampleRate;
		uint32_t m_channelCount;
		std::chrono::milliseconds m_duration;
	};

}