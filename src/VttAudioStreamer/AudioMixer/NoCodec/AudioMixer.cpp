#pragma once

#include "VttAudioStreamer/AudioMixer/NoCodec/AudioMixer.h"
#include <algorithm>
#include <chrono>
#include <thread>
#include <spdlog/spdlog.h>

namespace VttAudioStreamer::AudioMixer::NoCodec
{
	AudioMixer::AudioMixer()
	{
		
	}

	AudioMixer::~AudioMixer()
	{
		
	}

	void AudioMixer::FadeIn(const std::vector<std::shared_ptr<IAudioTrack>>& tracks, std::shared_ptr<IFade> fade)
	{
		if (!fade)
		{
			return;
		}

		std::lock_guard<std::mutex> lock(m_tracksMutex);

		for (const auto& track : tracks)
		{
			if (!track)
			{
				continue;
			}

			// Remove existing state for this track if it exists
			auto it = std::find_if(m_activeTracks.begin(), m_activeTracks.end(),
				[&track](const TrackState& state) { return state.track->IsEqual(track); });

			if (it != m_activeTracks.end())
			{
				m_activeTracks.erase(it);
			}

			// Add new track state with fade-in configuration
			TrackState state;
			state.track = track;
			state.targetVolume = track->GetVolume();
			state.currentVolume = 0.0f;
			state.fadeDuration = fade->GetDuration();
			state.elapsedTime = std::chrono::milliseconds(0);
			state.isActive = true;

			// Get PCM stream from audio sample
			auto audioSample = track->GetAudioSample();
			if (audioSample)
			{
				state.pcmStream = audioSample->GetPcmStream();
			}

			m_activeTracks.push_back(state);
			spdlog::debug("Track faded in: {}", audioSample ? audioSample->GetName() : "unknown");
		}
	}

	void AudioMixer::FadeOut(const std::vector<std::shared_ptr<IAudioTrack>>& tracks, std::shared_ptr<IFade> fade)
	{
		if (!fade)
		{
			return;
		}

		std::lock_guard<std::mutex> lock(m_tracksMutex);

		for (const auto& track : tracks)
		{
			if (!track)
			{
				continue;
			}

			auto it = std::find_if(m_activeTracks.begin(), m_activeTracks.end(),
				[&track](const TrackState& state) { return state.track->IsEqual(track); });

			if (it != m_activeTracks.end())
			{
				it->targetVolume = 0.0f;
				it->fadeDuration = fade->GetDuration();
				it->elapsedTime = std::chrono::milliseconds(0);
				spdlog::debug("Track faded out");
			}
		}
	}

	void AudioMixer::FadeInOut(const std::vector<std::shared_ptr<IAudioTrack>>& fromTracks,
		const std::vector<std::shared_ptr<IAudioTrack>>& toTracks,
		std::shared_ptr<ITransition> transition)
	{
		if (!transition)
		{
			return;
		}

		auto fadeOut = transition->GetFadeOut();
		auto fadeIn = transition->GetFadeIn();

		if (transition->IsOverlapping())
		{
			// Overlapping: both fades happen simultaneously
			FadeOut(fromTracks, fadeOut);
			FadeIn(toTracks, fadeIn);
		}
		else
		{
			// Sequential: fade out first, then fade in
			FadeOut(fromTracks, fadeOut);

			// Schedule fade in after fade out completes
			std::thread([this, toTracks, fadeIn]()
			{
				std::this_thread::sleep_for(fadeIn->GetDuration());
				FadeIn(toTracks, fadeIn);
			}).detach();
		}
	}

	void AudioMixer::FetchNextFrames()
	{
		std::lock_guard<std::mutex> lock(m_tracksMutex);

		for (auto& state : m_activeTracks)
		{
			if (!state.pcmStream)
			{
				continue;
			}

			// Fetch next frame if current is exhausted
			if (!state.currentFrame || state.currentSampleIndex >= state.currentFrame->GetSamples().size())
			{
				auto nextFramePromise = state.pcmStream->Next();
				try
				{
					state.currentFrame = nextFramePromise.get_future().get();
					state.currentSampleIndex = 0;
				}
				catch (const std::exception& e)
				{
					spdlog::warn("Failed to fetch next frame: {}", e.what());
					state.pcmStream = nullptr;
				}
			}
		}
	}

	std::vector<float> AudioMixer::MixAudioSamples()
	{
		std::vector<float> mixedSamples(FRAMES_PER_BUFFER * CHANNEL_COUNT, 0.0f);

		{
			std::lock_guard<std::mutex> lock(m_tracksMutex);

			for (auto it = m_activeTracks.begin(); it != m_activeTracks.end();)
			{
				auto& state = *it;

				// Update fade progress
				if (state.fadeDuration.count() > 0 && state.elapsedTime < state.fadeDuration)
				{
					float progress = static_cast<float>(state.elapsedTime.count()) / state.fadeDuration.count();
					state.currentVolume = (1.0f - progress) * state.currentVolume + progress * state.targetVolume;
				}
				else
				{
					state.currentVolume = state.targetVolume;
				}

				// Mix samples from current frame
				if (state.currentFrame)
				{
					const auto& frameSamples = state.currentFrame->GetSamples();
					size_t sampleIndex = state.currentSampleIndex;
					float volume = state.currentVolume * state.track->GetVolume();

					for (size_t i = 0; i < FRAMES_PER_BUFFER * CHANNEL_COUNT && sampleIndex < frameSamples.size(); ++i, ++sampleIndex)
					{
						mixedSamples[i] += frameSamples[sampleIndex] * volume;
					}

					state.currentSampleIndex = sampleIndex;
				}

				// Remove track if fade-out is complete and no more frames
				if (state.targetVolume == 0.0f && state.currentVolume < 0.001f && !state.currentFrame)
				{
					it = m_activeTracks.erase(it);
				}
				else
				{
					++it;
				}
			}
		}

		// Clamp samples to [-1.0, 1.0] to prevent clipping
		for (auto& sample : mixedSamples)
		{
			sample = std::clamp(sample, -1.0f, 1.0f);
		}

		return mixedSamples;
	}

	std::shared_ptr<IPcmConfig> AudioMixer::GetOutputPcmConfig() const
	{
		return m_PcmConfig;
	}

	std::shared_ptr<IPcmFrame> AudioMixer::GetPcmFrames(size_t frames)
	{
		auto frameDuration = std::chrono::milliseconds(frames * 1000 / SAMPLE_RATE);

		std::shared_ptr<IPcmFrame> outputFrame = nullptr;
		FetchNextFrames();

		auto mixedSamples = MixAudioSamples();

		if (!mixedSamples.empty())
		{
			outputFrame = std::make_shared<PcmFrame>(
				mixedSamples,
				m_PcmConfig
			);
		}

		// Update elapsed time for fade calculations
		{
			std::lock_guard<std::mutex> lock(m_tracksMutex);
			for (auto& state : m_activeTracks)
			{
				state.elapsedTime += frameDuration;
			}
		}


		return outputFrame;
	}
}