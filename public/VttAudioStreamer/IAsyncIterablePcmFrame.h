#pragma once

#include <future>
#include "VttAudioStreamer/IPcmFrame.h"
#include <memory>

namespace VttAudioStreamer
{
	class IAsyncIterablePcmFrame
	{
	public:
		virtual ~IAsyncIterablePcmFrame() = default;
		virtual std::promise<std::shared_ptr<IPcmFrame>> Next() const = 0;
	};
}