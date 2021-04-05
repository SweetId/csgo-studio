#pragma once

#include "common.h"

#include <type_traits>

enum class ECodec : uint32_t
{
	None,

	// Audio Codecs
	Mp2,

	// Video Codecs
	h264,

	MAX
};

enum class ESampleFormat : uint16_t
{
	None,

	U8,
	S16,
	S32,
	Float,
	Double,

	MAX
};
