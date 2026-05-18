#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include <cmath>
#include <atomic>
#include <algorithm>

#include "VttAudioStreamer/AudioMixer/NoCodec/AudioMixer.h"
#include "VttAudioStreamer/IAudioTrack.h"
#include "VttAudioStreamer/IAudioSample.h"
#include "VttAudioStreamer/IAsyncIterablePcmFrame.h"
#include "VttAudioStreamer/IPlaybackRange.h"
#include "VttAudioStreamer/IFade.h"
#include "VttAudioStreamer/PcmFrame.h"

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

using namespace VttAudioStreamer;
using namespace VttAudioStreamer::AudioMixer::NoCodec;



// Helper classes for test app
class PlaybackRange : public IPlaybackRange
{
public:
	PlaybackRange(std::chrono::milliseconds start, std::chrono::milliseconds end)
		: m_start(start), m_end(end)
	{
	}

	std::chrono::milliseconds GetStart() const override { return m_start; }
	std::chrono::milliseconds GetEnd() const override { return m_end; }

private:
	std::chrono::milliseconds m_start;
	std::chrono::milliseconds m_end;
};

class SineWaveGenerator : public IAsyncIterablePcmFrame
{
public:
	SineWaveGenerator(float frequency, uint32_t sampleRate, uint32_t channelCount,
		std::chrono::milliseconds duration)
		: m_frequency(frequency)
		, m_sampleRate(sampleRate)
		, m_channelCount(channelCount)
		, m_duration(duration)
		, m_currentSample(0)
		, m_totalSamples((sampleRate* duration.count()) / 1000)
	{
	}

	std::promise<std::shared_ptr<IPcmFrame>> Next() const override
	{
		std::promise<std::shared_ptr<IPcmFrame>> promise;

		try
		{
			const size_t FRAME_SIZE = 735; // ~16ms at 44100Hz
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
				float sineValue = std::sin(phase) * 0.5f;

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

class AudioSample : public IAudioSample
{
public:
	AudioSample(const std::string& name, float frequency, uint32_t sampleRate,
		uint32_t channelCount, std::chrono::milliseconds duration)
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
		return std::make_shared<PlaybackRange>(std::chrono::milliseconds(0), m_duration);
	}

	std::shared_ptr<IAsyncIterablePcmFrame> GetPcmStream() const override
	{
		return std::make_shared<SineWaveGenerator>(m_frequency, m_sampleRate, m_channelCount, m_duration);
	}

private:
	std::string m_name;
	float m_frequency;
	uint32_t m_sampleRate;
	uint32_t m_channelCount;
	std::chrono::milliseconds m_duration;
};

class AudioTrack : public IAudioTrack
{
public:
	AudioTrack(const std::string& name, float frequency, uint32_t sampleRate,
		uint32_t channelCount, std::chrono::milliseconds duration, float volume = 1.0f)
		: m_name(name)
		, m_volume(volume)
		, m_audioSample(std::make_shared<AudioSample>(name, frequency, sampleRate, channelCount, duration))
	{
	}

	std::shared_ptr<IAudioSample> GetAudioSample() const override { return m_audioSample; }
	float GetVolume() const override { return m_volume; }
	bool IsEqual(const std::shared_ptr<IAudioTrack>& other) const override { return this == other.get(); }

private:
	std::string m_name;
	float m_volume;
	std::shared_ptr<IAudioSample> m_audioSample;
};

class Fade : public IFade
{
public:
	explicit Fade(std::chrono::milliseconds duration) : m_duration(duration) {}
	std::chrono::milliseconds GetDuration() const override { return m_duration; }

private:
	std::chrono::milliseconds m_duration;
};

// Audio device callback for miniaudio
struct AudioContext
{
	ma_pcm_rb ringBuffer;
	std::atomic<bool> isRunning = false;
};

void DataCallback(ma_device* device, void* output, const void* input, ma_uint32 frameCount)
{
	AudioContext* context = static_cast<AudioContext*>(device->pUserData);
	ma_uint32 framesAcquired;
	void* buffer;
	if (ma_pcm_rb_acquire_read(&context->ringBuffer, &framesAcquired, &buffer) != MA_SUCCESS)
	{
		spdlog::get("VttAudioStreamer.AudioMixer.TestApp")->error("Failed to acquire read from ring buffer");
		return;
	}

	ma_uint32 framesToWrite = (framesAcquired < frameCount) ? framesAcquired : frameCount;
	memcpy(output, buffer, framesToWrite * device->playback.channels * sizeof(float));

	if (framesAcquired < frameCount)
	{
		spdlog::get("VttAudioStreamer.AudioMixer.TestApp")->warn("Audio underrun: requested {} frames, got {} frames",	frameCount, framesAcquired);
	}

	if (ma_pcm_rb_commit_read(&context->ringBuffer, framesToWrite) != MA_SUCCESS)
	{
		spdlog::get("VttAudioStreamer.AudioMixer.TestApp")->error("Failed to commit read to ring buffer");
		return;
	}

}

int main()
{
	auto logger = spdlog::stdout_color_mt("VttAudioStreamer.AudioMixer.TestApp");
	spdlog::set_default_logger(logger);
	spdlog::set_level(spdlog::level::info);

	logger->info("VttAudioStreamer Test Application");
	logger->info("==================================");

	try
	{
		// Initialize miniaudio device
		ma_device_config deviceConfig = ma_device_config_init(ma_device_type_playback);
		deviceConfig.playback.format = ma_format_f32;
		deviceConfig.playback.channels = 2;
		deviceConfig.sampleRate = 44100;
		deviceConfig.dataCallback = DataCallback;

		AudioContext context;
		deviceConfig.pUserData = &context;

		ma_device device;
		if (ma_device_init(nullptr, &deviceConfig, &device) != MA_SUCCESS)
		{
			logger->error("Failed to initialize audio device");
			return 1;
		}

		// Create ring buffer for audio samples
		constexpr ma_uint32 BUFFER_SIZE_IN_FRAMES = 44100 * 2; // 2 seconds of buffered audio
		if (ma_pcm_rb_init(device.playback.format, device.playback.channels, BUFFER_SIZE_IN_FRAMES, nullptr, nullptr, &context.ringBuffer) != MA_SUCCESS)
		{
			logger->error("Failed to create ring buffer");
			ma_device_uninit(&device);
			return 1;
		}

		// Create and start the AudioMixer
		VttAudioStreamer::AudioMixer::NoCodec::AudioMixer mixer;
		context.isRunning = true;

		// Set frame callback to feed audio to miniaudio
		mixer.SetOnFrameCallback([&context, &device, &logger](std::shared_ptr<IPcmFrame> frame)
			{
				if (!frame)
				{
					return;
				}

				auto samples = frame->GetSamples();
				ma_uint32 framesAvailable;
				void* buffer;
				if (ma_pcm_rb_acquire_write(
					&context.ringBuffer,
					&framesAvailable,
					&buffer
				) != MA_SUCCESS)
				{
					logger->error("ma_pcm_rb_acquire_write() failed");
					return;
				}

				ma_uint32 valuesSpaceAvailable = framesAvailable * device.playback.channels;
				ma_uint32 valuesToWrite = std::min<ma_uint32>(valuesSpaceAvailable,	static_cast<ma_uint32>(samples.size()));

				memcpy(buffer, samples.data(), valuesToWrite * sizeof(float));

				if (valuesSpaceAvailable < valuesToWrite)
				{
					logger->warn("Ring buffer full, dropping {} frames",
						(samples.size() - valuesToWrite) / device.playback.channels);
				}
				auto framesWritten = valuesToWrite / device.playback.channels;
				// logger->info("Committing {} written frames", framesWritten);
				if(ma_pcm_rb_commit_write(&context.ringBuffer, framesWritten) != MA_SUCCESS)
				{
					logger->error("ma_pcm_rb_commit_write() failed");
				}
			});

		// Start mixer
		auto startPromise = mixer.Start();
		startPromise.get_future().get();
		logger->info("AudioMixer started");

		// Start audio device
		if (ma_device_start(&device) != MA_SUCCESS)
		{
			logger->error("Failed to start audio device");
			mixer.Stop().get_future().get();
			ma_pcm_rb_uninit(&context.ringBuffer);
			ma_device_uninit(&device);
			return 1;
		}
		logger->info("Audio device started");

		// Create and play audio tracks
		auto track1 = std::make_shared<AudioTrack>("Sine_440Hz", 440.0f, 44100, 2, std::chrono::milliseconds(3000), 0.8f);
		auto track2 = std::make_shared<AudioTrack>("Sine_550Hz", 550.0f, 44100, 2, std::chrono::milliseconds(3000), 0.6f);

		auto fadeIn = std::make_shared<Fade>(std::chrono::milliseconds(500));
		auto fadeOut = std::make_shared<Fade>(std::chrono::milliseconds(500));

		// Play first track
		logger->info("Playing 440Hz sine wave (A4) for 3 seconds...");
		mixer.FadeIn({ track1 }, fadeIn);
		std::this_thread::sleep_for(std::chrono::milliseconds(2000));

		// Add second track
		logger->info("Adding 550Hz sine wave (C#5)...");
		mixer.FadeIn({ track2 }, fadeIn);
		std::this_thread::sleep_for(std::chrono::milliseconds(1500));

		// Fade out both
		logger->info("Fading out...");
		mixer.FadeOut({ track1, track2 }, fadeOut);
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));

		// Stop mixer and device
		logger->info("Stopping audio mixer...");
		mixer.Stop().get_future().get();

		ma_device_stop(&device);
		logger->info("Audio device stopped");

		// Cleanup
		ma_pcm_rb_uninit(&context.ringBuffer);
		ma_device_uninit(&device);

		logger->info("Test completed successfully");
		return 0;
	}
	catch (const std::exception& e)
	{
		spdlog::get("VttAudioStreamer.AudioMixer.TestApp")->error("Exception: {}", e.what());
		return 1;
	}
}