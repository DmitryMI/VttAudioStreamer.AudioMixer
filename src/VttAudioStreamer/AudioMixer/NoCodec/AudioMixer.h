#pragma once

#include "VttAudioStreamer/AudioMixer/IAudioMixer.h"
#include "VttAudioStreamer/PcmFrame.h"
#include <thread>

namespace VttAudioStreamer::AudioMixer::NoCodec
{
	namespace
	{
		struct TrackState
		{
			std::shared_ptr<IAudioTrack> track;
			float targetVolume = 1.0f;
			float currentVolume = 0.0f;
			std::chrono::milliseconds fadeDuration = std::chrono::milliseconds(0);
			std::chrono::milliseconds elapsedTime = std::chrono::milliseconds(0);
			bool isActive = false;
			std::shared_ptr<IAsyncIterablePcmFrame> pcmStream;
			std::shared_ptr<IPcmFrame> currentFrame;
			size_t currentSampleIndex = 0;
		};
	}

	class AudioMixer : public IAudioMixer
	{
	public:
		AudioMixer();
		virtual ~AudioMixer();

		std::promise<void> Start() override;
		std::promise<void> Stop() override;
		void FadeIn(const std::vector<std::shared_ptr<IAudioTrack>>& tracks, std::shared_ptr<IFade> fade) override;
		void FadeOut(const std::vector<std::shared_ptr<IAudioTrack>>& tracks, std::shared_ptr<IFade> fade) override;
		void FadeInOut(const std::vector<std::shared_ptr<IAudioTrack>>& fromTracks, const std::vector<std::shared_ptr<IAudioTrack>>& toTracks, std::shared_ptr<ITransition> transition) override;

		void SetOnFrameCallback(OnFrameCallback callback) override;

	private:
		void MixingLoop();
		std::vector<float> MixAudioSamples();
		void FetchNextFrames();

		OnFrameCallback m_OnFrameCallback;
		bool m_isRunning = false;
		std::thread m_mixerThread;
		std::vector<TrackState> m_activeTracks;
		std::mutex m_tracksMutex;
		std::mutex m_callbackMutex;

		// Audio format configuration
		static constexpr int SAMPLE_RATE = 44100;
		static constexpr int CHANNEL_COUNT = 2;
		static constexpr int FRAMES_PER_BUFFER = 735; // ~16ms at 44100Hz
	};
}