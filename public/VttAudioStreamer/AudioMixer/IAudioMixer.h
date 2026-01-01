#pragma once

#include <functional>
#include <future>
#include <memory>
#include <vector>
#include "VttAudioStreamer/IAudioTrack.h"
#include "VttAudioStreamer/IFade.h"
#include "VttAudioStreamer/IPcmFrame.h"
#include "VttAudioStreamer/ITransition.h"

namespace VttAudioStreamer::AudioMixer
{
	/// <summary>
	/// Mixes multiple audio tracks into a single output stream.
	/// </summary>
	class IAudioMixer
	{
	public:
		/// <summary>
		/// Callback invoked when a new mixed audio frame is available.
		/// </summary>
		using OnFrameCallback = std::function<void(std::shared_ptr<IPcmFrame>)>;

		virtual ~IAudioMixer() = default;

		/// <summary>
		/// Starts the audio mixer.
		/// </summary>
		virtual std::promise<void> Start() = 0;

		/// <summary>
		/// Stops the audio mixer.
		/// </summary>
		virtual std::promise<void> Stop() = 0;

		virtual void FadeIn(const std::vector<std::shared_ptr<IAudioTrack>>& tracks, std::shared_ptr<IFade> fade) = 0;
		virtual void FadeOut(const std::vector<std::shared_ptr<IAudioTrack>>& tracks, std::shared_ptr<IFade> fade) = 0;
		virtual void FadeInOut(const std::vector<std::shared_ptr<IAudioTrack>>& fromTracks, const std::vector<std::shared_ptr<IAudioTrack>>& toTracks, std::shared_ptr<ITransition> transition) = 0;

		virtual void SetOnFrameCallback(OnFrameCallback callback) = 0;
	};
}