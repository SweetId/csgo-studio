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

namespace
{
	template<typename TEnum>
	inline typename std::underlying_type<TEnum>::type EnumCast(const TEnum e) { return std::underlying_type<TEnum>::type(e); }

	template<typename TEnum>
	inline typename TEnum EnumCast(const typename std::underlying_type<TEnum>::type v) { return TEnum(v); }
}