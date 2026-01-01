#pragma once

#include "VttAudioStreamer/AudioMixer/Gstreamer/AudioMixer.h"

namespace VttAudioStreamer::AudioMixer::Gstreamer
{
	std::promise<void> AudioMixer::Start()
	{
		return {};
	}

	std::promise<void> AudioMixer::Stop()
	{
		return {};
	}

	void AudioMixer::FadeIn(const std::vector<std::shared_ptr<IAudioTrack>>& tracks, std::shared_ptr<IFade> fade)
	{

	}

	void AudioMixer::FadeOut(const std::vector<std::shared_ptr<IAudioTrack>>& tracks, std::shared_ptr<IFade> fade)
	{

	}

	void AudioMixer::FadeInOut(const std::vector<std::shared_ptr<IAudioTrack>>& fromTracks, const std::vector<std::shared_ptr<IAudioTrack>>& toTracks, std::shared_ptr<ITransition> transition)
	{
	}

	void AudioMixer::SetOnFrameCallback(OnFrameCallback callback)
	{
		m_OnFrameCallback = callback;
	}
}