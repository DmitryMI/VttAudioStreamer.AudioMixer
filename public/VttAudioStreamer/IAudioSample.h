#pragma once

#include "VttAudioStreamer/IPlaybackRange.h"
#include "VttAudioStreamer/IAsyncIterablePcmFrame.h"

namespace VttAudioStreamer
{
	class IAudioSample
	{
	public:
		virtual ~IAudioSample() = default;
		virtual std::string GetName() const = 0;
		virtual std::shared_ptr<IPlaybackRange> GetPlaybackRange() const = 0;
		virtual std::shared_ptr<IAsyncIterablePcmFrame> GetPcmStream() const = 0;

	};
}