#pragma once

#include "common.h"
#include "callback.h"

#include "audio_sample_descriptor.h"

class CSGOSTUDIO_API StreamEncoder
{
public:
	StreamEncoder(const AudioSampleDescriptor& descriptor);
	~StreamEncoder();

	bool Encode(const uint32_t size, const uint8_t* data);

	Callback<const uint32_t, const uint8_t*> DataAvailable;

private:
	bool Initialize();
	// Encode current frame in the packet
	bool EncodeInternal(struct AVFrame* frame);

	AudioSampleDescriptor m_descriptor;

	struct AVFrame* m_frame;
};