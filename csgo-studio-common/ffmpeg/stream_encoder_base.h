#pragma once

#include "common.h"

#include <QByteArray>

class CSGOSTUDIO_API StreamEncoderBase
{
public:
	StreamEncoderBase();
	virtual ~StreamEncoderBase();

	bool Encode(const class QByteArray& inBuffer, QByteArray& outBuffer);

protected:
	bool Initialize();
	// Encode current frame in the packet
	bool EncodeInternal(struct AVFrame* frame, QByteArray& outBuffer);

	virtual struct SampleDescriptorBase& GetDescriptor() = 0;
	virtual bool OnInitialize() = 0;
	virtual int32_t GetFrameSize() const = 0;
	virtual void CopyBufferToFrame() = 0;

	struct AVFrame* m_frame;

	QByteArray m_internalBuffer;
};