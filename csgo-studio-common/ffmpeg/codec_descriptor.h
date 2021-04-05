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

	struct AVCodec* GetEncoder() { return m_privateEncoder; }
	struct AVCodec* GetDecoder() { return m_privateDecoder; }

	std::set<int32_t> SupportedEncodingSampleRates() const;
	std::set<int32_t> SupportedEncodingChannels() const;
	std::set<ESampleFormat> SupportedEncodingFormats() const;
	uint64_t GetFirstLayoutForChannels(uint64_t channels) const;

	std::set<int32_t> SupportedDecodingSampleRates() const;
	std::set<int32_t> SupportedDecodingChannels() const;
	std::set<ESampleFormat> SupportedDecodingFormats() const;

private:
	ECodec m_codec;

	// No need for a specific copy ctor as codec is returned by ffmpeg
	// and does not need to be freed manually
	struct AVCodec* m_privateEncoder;
	struct AVCodec* m_privateDecoder;
};