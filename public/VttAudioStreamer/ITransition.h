#pragma once

#include "VttAudioStreamer/IFade.h"

namespace VttAudioStreamer
{
	class ITransition
	{
	public:
		virtual ~ITransition() = default;

		virtual bool IsOverlapping() const = 0;
		virtual std::shared_ptr<IFade> GetFadeOut() const = 0;
		virtual std::shared_ptr<IFade> GetFadeIn() const = 0;
	};
}