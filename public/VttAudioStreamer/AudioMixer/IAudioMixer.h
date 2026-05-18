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

		virtual ~IAudioMixer() = default;

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

		virtual std::shared_ptr<IPcmConfig> GetOutputPcmConfig() const = 0;

		/// <summary>
		/// Mixes and retrieves a specified number of PCM audio frames.
		/// </summary>
		/// <param name="frames">The number of PCM frames to retrieve.</param>
		/// <returns>A shared pointer to an IPcmFrame object containing the requested frames. May contain less frames then requested.</returns>
		virtual std::shared_ptr<IPcmFrame> GetPcmFrames(size_t frames) = 0;
	};
}