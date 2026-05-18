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

		void FadeIn(const std::vector<std::shared_ptr<IAudioTrack>>& tracks, std::shared_ptr<IFade> fade) override;
		void FadeOut(const std::vector<std::shared_ptr<IAudioTrack>>& tracks, std::shared_ptr<IFade> fade) override;
		void FadeInOut(const std::vector<std::shared_ptr<IAudioTrack>>& fromTracks, const std::vector<std::shared_ptr<IAudioTrack>>& toTracks, std::shared_ptr<ITransition> transition) override;

		std::shared_ptr<IPcmConfig> GetOutputPcmConfig() const override;
		std::shared_ptr<IPcmFrame> GetPcmFrames(size_t frames) override;

	private:
		std::vector<float> MixAudioSamples();
		void FetchNextFrames();

		bool m_isRunning = false;
		std::vector<TrackState> m_activeTracks;
		std::mutex m_tracksMutex;

		std::shared_ptr<IPcmConfig> m_PcmConfig = std::make_shared<PcmConfig>(SAMPLE_RATE, CHANNEL_COUNT);

		// Audio format configuration
		static constexpr int SAMPLE_RATE = 44100;
		static constexpr int CHANNEL_COUNT = 2;
		static constexpr int FRAMES_PER_BUFFER = 735; // ~16ms at 44100Hz
	};
}