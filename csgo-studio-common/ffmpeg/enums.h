#pragma once

#include "common.h"

enum class ECodec : uint32_t
{
	None,

	// Audio Codecs
	Mp2

	// Video Codecs
};

enum class ESampleFormat : uint16_t
{
	None,

	U8,
	S16,
	S32,
	Float,
	Double
};
