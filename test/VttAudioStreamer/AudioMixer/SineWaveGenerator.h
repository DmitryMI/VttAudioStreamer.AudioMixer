#pragma once

#include "VttAudioStreamer/IAsyncIterablePcmFrame.h"

namespace VttAudioStreamer
{

	// Mock implementation of IAsyncIterablePcmFrame that generates a sine wave
	class SineWaveGenerator : public IAsyncIterablePcmFrame
	{
	public:
		SineWaveGenerator(
			float frequency,
			uint32_t sampleRate,
			uint32_t channelCount,
			std::chrono::milliseconds duration)
			: m_frequency(frequency)
			, m_sampleRate(sampleRate)
			, m_channelCount(channelCount)
			, m_duration(duration)
			, m_currentSample(0)
		{
			m_totalSamples = (sampleRate * duration.count()) / 1000;
		}

		std::promise<std::shared_ptr<IPcmFrame>> Next() const override
		{
			std::promise<std::shared_ptr<IPcmFrame>> promise;

			try
			{
				// Generate frame of 735 samples (~16ms at 44100Hz)
				const size_t FRAME_SIZE = 735;
				std::vector<float> samples;
				samples.reserve(FRAME_SIZE * m_channelCount);

				const float phaseIncrement = 2.0f * 3.14159265359f * m_frequency / m_sampleRate;

				for (size_t i = 0; i < FRAME_SIZE; ++i)
				{
					if (m_currentSample >= m_totalSamples)
					{
						break;
					}

					float phase = phaseIncrement * m_currentSample;
					float sineValue = std::sin(phase) * 0.5f; // 0.5 amplitude to prevent clipping

					// Add same sample to all channels
					for (uint32_t ch = 0; ch < m_channelCount; ++ch)
					{
						samples.push_back(sineValue);
					}

					++m_currentSample;
				}

				if (samples.empty())
				{
					promise.set_exception(std::make_exception_ptr(
						std::runtime_error("No more samples available")));
				}
				else
				{
					auto timestamp = std::chrono::milliseconds(
						(m_currentSample - samples.size() / m_channelCount) * 1000 / m_sampleRate);
					auto frame = std::make_shared<PcmFrame>(samples, m_sampleRate, m_channelCount, timestamp);
					promise.set_value(frame);
				}
			}
			catch (const std::exception&)
			{
				promise.set_exception(std::current_exception());
			}

			return promise;
		}

	private:
		mutable std::atomic<size_t> m_currentSample;
		float m_frequency;
		uint32_t m_sampleRate;
		uint32_t m_channelCount;
		std::chrono::milliseconds m_duration;
		size_t m_totalSamples;
	};

}