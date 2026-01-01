#pragma once

#include "VttAudioStreamer/IAudioSample.h"

namespace VttAudioStreamer
{
	class IAudioTrack
	{
	public:
		virtual std::shared_ptr<IAudioSample> GetAudioSample() const = 0;
		virtual float GetVolume() const = 0;
	};
}