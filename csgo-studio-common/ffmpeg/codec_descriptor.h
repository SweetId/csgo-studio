#pragma once

#include "common.h"

#include "enums.h"

#include <set>

struct CSGOSTUDIO_API CodecDescriptor
{
public:
	CodecDescriptor(ECodec codec = ECodec::None);
	~CodecDescriptor();

	bool IsValid() const;

	bool SetCodec(ECodec codec);
	ECodec GetCodec() const { return m_codec; }

	std::set<int32_t> SupportedSampleRates() const;
	std::set<int32_t> SupportedChannels() const;
	std::set<ESampleFormat> SupportedFormats() const;
	uint64_t GetFirstLayoutForChannels(uint64_t channels) const;

private:
	ECodec m_codec;

	// No need for a specific copy ctor as codec is returned by ffmpeg
	// and does not need to be freed manually
	struct AVCodec* m_privateCodec;

	friend struct AudioSampleDescriptor;
	friend class StreamEncoder;
	friend class StreamDecoder;
};