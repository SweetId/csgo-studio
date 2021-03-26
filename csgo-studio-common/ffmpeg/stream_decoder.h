#pragma once

#include "common.h"
#include "callback.h"

#include "audio_sample_descriptor.h"

class CSGOSTUDIO_API StreamDecoder
{
public:
	StreamDecoder(const AudioSampleDescriptor& descriptor);
	~StreamDecoder();

	bool Decode(const class QByteArray& inBuffer, class QByteArray& outBuffer);

private:
	bool Initialize();
	// Encode current frame in the packet
	bool DecodeInternal(struct AVPacket* packet, class QByteArray& outBuffer);

	AudioSampleDescriptor m_descriptor;

	struct AVFrame* m_frame;
	struct AVCodecParserContext* m_parser;
};