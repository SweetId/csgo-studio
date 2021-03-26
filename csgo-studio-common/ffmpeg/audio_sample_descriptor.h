#pragma once

#include "common.h"

#include "codec_descriptor.h"

struct CSGOSTUDIO_API AudioSampleDescriptor
{
public:
	AudioSampleDescriptor();
	AudioSampleDescriptor(ECodec codec, int32_t channels, int32_t bitRate, int32_t sampleRate, ESampleFormat format);
	~AudioSampleDescriptor();

	AudioSampleDescriptor(const AudioSampleDescriptor& o);
	AudioSampleDescriptor& operator=(const AudioSampleDescriptor& o);

	bool IsValid() const;

	bool SetCodec(ECodec codec);
	bool SetChannels(int32_t channels);
	bool SetBitRate(int32_t rate);
	bool SetSampleRate(int32_t rate);
	bool SetSampleFormat(ESampleFormat format);

private:
	bool UpdateInternal();
	void DeleteContext();

	ESampleFormat m_format;
	int32_t m_bitRate;
	int32_t m_sampleRate;
	int32_t m_channels;

	CodecDescriptor m_codec;

	struct AVCodecContext* m_privateEncodingContext;
	struct AVCodecContext* m_privateDecodingContext;

	friend class StreamEncoder;
	friend class StreamDecoder;
};