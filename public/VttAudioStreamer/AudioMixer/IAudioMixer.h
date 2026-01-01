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

		/// <summary>
		/// Fades in the specified audio tracks using the given fade configuration, if they are not already playing.
		/// </summary>
		/// <param name="tracks">A collection of audio tracks to fade in.</param>
		/// <param name="fade">The fade configuration that defines the fade-in behavior.</param>
		virtual void FadeIn(const std::vector<std::shared_ptr<IAudioTrack>>& tracks, std::shared_ptr<IFade> fade) = 0;

		/// <summary>
		/// Fades out the specified audio tracks using the given fade configuration, if they are currently playing.
		/// </summary>
		/// <param name="tracks"></param>
		/// <param name="fade"></param>
		virtual void FadeOut(const std::vector<std::shared_ptr<IAudioTrack>>& tracks, std::shared_ptr<IFade> fade) = 0;

		/// <summary>
		/// Fades out the specified "from" audio tracks and fades in the specified "to" audio tracks using the given transition configuration.
		/// </summary>
		/// <param name="fromTracks"></param>
		/// <param name="toTracks"></param>
		/// <param name="transition"></param>
		virtual void FadeInOut(const std::vector<std::shared_ptr<IAudioTrack>>& fromTracks, const std::vector<std::shared_ptr<IAudioTrack>>& toTracks, std::shared_ptr<ITransition> transition) = 0;

		/// <summary>
		/// Sets a callback function to be invoked on each frame.
		/// </summary>
		/// <param name="callback">The callback function to be called when a frame occurs.</param>
		virtual void SetOnFrameCallback(OnFrameCallback callback) = 0;
	};
}