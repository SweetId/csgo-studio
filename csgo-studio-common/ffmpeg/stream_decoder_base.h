#pragma once

#include "common.h"

#include <QByteArray>

class CSGOSTUDIO_API StreamDecoderBase
{
public:
	StreamDecoderBase();
	virtual ~StreamDecoderBase();

	bool Decode(const class QByteArray& inBuffer, QByteArray& outBuffer);

protected:
	bool Initialize();

	// Encode current frame in the packet
	bool DecodeInternal(struct AVPacket* packet, QByteArray& outBuffer);

	virtual struct SampleDescriptorBase& GetDescriptor() = 0;
	virtual int32_t GetFrameSize() const = 0;
	virtual void CopyFrameToBuffer(struct AVFrame* frame, QByteArray& outBuffer) = 0;

	struct AVCodecParserContext* m_parser;

	QByteArray m_internalBuffer;
};