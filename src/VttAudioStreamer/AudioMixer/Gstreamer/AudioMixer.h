#pragma once

#include "VttAudioStreamer/AudioMixer/IAudioMixer.h"

namespace VttAudioStreamer::AudioMixer::Gstreamer
{
	class AudioMixer : public IAudioMixer
	{
	public:
		AudioMixer() = default;
		virtual ~AudioMixer() = default;

		std::promise<void> Start() override;
		std::promise<void> Stop() override;
		void FadeIn(const std::vector<std::shared_ptr<IAudioTrack>>& tracks, std::shared_ptr<IFade> fade) override;
		void FadeOut(const std::vector<std::shared_ptr<IAudioTrack>>& tracks, std::shared_ptr<IFade> fade) override;
		void FadeInOut(const std::vector<std::shared_ptr<IAudioTrack>>& fromTracks, const std::vector<std::shared_ptr<IAudioTrack>>& toTracks, std::shared_ptr<ITransition> transition) override;

		void SetOnFrameCallback(OnFrameCallback callback) override;

	private:
		OnFrameCallback m_OnFrameCallback;
	};
}