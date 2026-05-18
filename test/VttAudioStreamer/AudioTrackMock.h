#pragma once 

#include "VttAudioStreamer/IAudioSample.h"

namespace VttAudioStreamer
{

	// Mock implementation of IAudioTrack
	class AudioTrackMock : public IAudioTrack
	{
	public:
		AudioTrackMock(
			const std::string& name,
			float frequency,
			uint32_t sampleRate,
			uint32_t channelCount,
			std::chrono::milliseconds duration,
			float volume = 1.0f)
			: m_name(name)
			, m_volume(volume)
			, m_audioSample(std::make_shared<AudioSampleMock>(name, frequency, sampleRate, channelCount, duration))
		{
		}

		std::shared_ptr<IAudioSample> GetAudioSample() const override { return m_audioSample; }
		float GetVolume() const override { return m_volume; }
		bool IsEqual(const std::shared_ptr<IAudioTrack>& other) const override
		{
			return this == other.get();
		}

	private:
		std::string m_name;
		float m_volume;
		std::shared_ptr<IAudioSample> m_audioSample;
	};

}