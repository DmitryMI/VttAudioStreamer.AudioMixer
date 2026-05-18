#include <gtest/gtest.h>
#include <cmath>
#include <memory>
#include <atomic>

#include "VttAudioStreamer/AudioMixer/NoCodec/AudioMixer.h"
#include "VttAudioStreamer/PcmFrame.h"
#include "VttAudioStreamer/PlaybackRangeMock.h"
#include "VttAudioStreamer/AudioMixer/SineWaveGenerator.h"
#include "VttAudioStreamer/AudioSampleMock.h"
#include "VttAudioStreamer/AudioTrackMock.h"
#include "VttAudioStreamer/FadeMock.h"

namespace VttAudioStreamer::AudioMixer::NoCodec
{
	// Basic construction test
	TEST(AudioMixerTest, ConstructTest)
	{
		AudioMixer mixer;
	}

	// Test playing a single sine wave through the mixer
	TEST(AudioMixerTest, PlaySingleSineWave)
	{
		AudioMixer mixer;

		// Create a sine wave track (440Hz - musical note A4)
		const float FREQUENCY = 440.0f;
		const uint32_t SAMPLE_RATE = 44100;
		const uint32_t CHANNEL_COUNT = 2;
		const std::chrono::milliseconds DURATION(2000); // 2 seconds

		auto sineTrack = std::make_shared<AudioTrackMock>(
			"SineWave_440Hz",
			FREQUENCY,
			SAMPLE_RATE,
			CHANNEL_COUNT,
			DURATION,
			1.0f);

		// Track received frames
		std::atomic<int> frameCount(0);
		std::atomic<float> maxAmplitude(0.0f);
		std::atomic<bool> callbackInvoked(false);

		// Set up frame callback to verify audio output
		mixer.SetOnFrameCallback([&](std::shared_ptr<IPcmFrame> frame)
		{
			callbackInvoked = true;
			frameCount.fetch_add(1, std::memory_order_relaxed);

			EXPECT_EQ(frame->GetSampleRate(), SAMPLE_RATE);
			EXPECT_EQ(frame->GetChannelCount(), CHANNEL_COUNT);

			// Verify we're getting samples
			const auto& samples = frame->GetSamples();
			EXPECT_FALSE(samples.empty());

			// Track maximum amplitude
			for (float sample : samples)
			{
				float absSample = std::abs(sample);
				float currentMax = maxAmplitude.load(std::memory_order_relaxed);
				while (absSample > currentMax && 
					   !maxAmplitude.compare_exchange_strong(currentMax, absSample, 
					   std::memory_order_relaxed))
				{
					currentMax = maxAmplitude.load(std::memory_order_relaxed);
				}
			}
		});

		// Create fade-in configuration
		auto fadeIn = std::make_shared<MockFade>(std::chrono::milliseconds(500));

		// Start the mixer
		auto startPromise = mixer.Start();
		EXPECT_NO_THROW(startPromise.get_future().get());

		// Wait a moment for the mixer to initialize
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		// Fade in the sine wave track
		mixer.FadeIn({ sineTrack }, fadeIn);

		// Let it play for 2 seconds
		std::this_thread::sleep_for(DURATION);

		// Fade out
		auto fadeOut = std::make_shared<MockFade>(std::chrono::milliseconds(500));
		mixer.FadeOut({ sineTrack }, fadeOut);

		// Wait for fade out to complete
		std::this_thread::sleep_for(std::chrono::milliseconds(600));

		// Stop the mixer
		auto stopPromise = mixer.Stop();
		EXPECT_NO_THROW(stopPromise.get_future().get());

		// Verify results
		EXPECT_TRUE(callbackInvoked);
		EXPECT_GT(frameCount, 0);
		EXPECT_GT(maxAmplitude, 0.1f); // Sine wave should have significant amplitude

		// Rough estimate: 2 seconds at 44100Hz with 735 samples per frame ~= 120 frames
		// Plus fade time, expect at least 100 frames
		EXPECT_GE(frameCount, 50);
	}

	// Test fade in effect
	TEST(AudioMixerTest, FadeInEffect)
	{
		AudioMixer mixer;

		const float FREQUENCY = 220.0f; // A3
		const uint32_t SAMPLE_RATE = 44100;
		const uint32_t CHANNEL_COUNT = 2;
		const std::chrono::milliseconds DURATION(3000);
		const std::chrono::milliseconds FADE_DURATION(500);

		auto sineTrack = std::make_shared<AudioTrackMock>(
			"FadeInTest",
			FREQUENCY,
			SAMPLE_RATE,
			CHANNEL_COUNT,
			DURATION,
			1.0f);

		std::vector<float> amplitudes;
		std::mutex amplitudeMutex;

		mixer.SetOnFrameCallback([&](std::shared_ptr<IPcmFrame> frame)
		{
			const auto& samples = frame->GetSamples();
			if (!samples.empty())
			{
				float frameMax = 0.0f;
				for (float sample : samples)
				{
					frameMax = std::max(frameMax, std::abs(sample));
				}
				{
					std::lock_guard<std::mutex> lock(amplitudeMutex);
					amplitudes.push_back(frameMax);
				}
			}
		});

		auto fadeIn = std::make_shared<MockFade>(FADE_DURATION);

		auto startPromise = mixer.Start();
		EXPECT_NO_THROW(startPromise.get_future().get());

		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		mixer.FadeIn({ sineTrack }, fadeIn);

		std::this_thread::sleep_for(DURATION);

		auto stopPromise = mixer.Stop();
		EXPECT_NO_THROW(stopPromise.get_future().get());

		// Verify fade-in behavior: amplitude should increase during fade period
		EXPECT_GE(amplitudes.size(), 5);
		if (amplitudes.size() >= 2)
		{
			// First few frames should have lower amplitude than later frames
			float earlyAverage = 0.0f;
			for (size_t i = 0; i < std::min(size_t(5), amplitudes.size()); ++i)
			{
				earlyAverage += amplitudes[i];
			}
			earlyAverage /= std::min(size_t(5), amplitudes.size());

			float lateAverage = 0.0f;
			size_t lateCount = std::min(size_t(10), amplitudes.size());
			for (size_t i = amplitudes.size() - lateCount; i < amplitudes.size(); ++i)
			{
				lateAverage += amplitudes[i];
			}
			lateAverage /= lateCount;

			EXPECT_LT(earlyAverage, lateAverage);
		}
	}
}