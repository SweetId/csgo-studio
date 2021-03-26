#pragma once

#include "common.h"
#include "callback.h"

#include "audio_sample_descriptor.h"

class CSGOSTUDIO_API StreamEncoder
{
public:
	StreamEncoder(const AudioSampleDescriptor& descriptor);
	~StreamEncoder();

	bool Encode(const class QByteArray& inBuffer, class QByteArray& outBuffer);

private:
	bool Initialize();
	// Encode current frame in the packet
	bool EncodeInternal(struct AVFrame* frame, class QByteArray& outBuffer);

	AudioSampleDescriptor m_descriptor;

	struct AVFrame* m_frame;
};