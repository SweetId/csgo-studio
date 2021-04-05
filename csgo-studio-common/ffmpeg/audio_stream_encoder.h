#pragma once

#include "common.h"
#include "audio_sample_descriptor.h"
#include "stream_encoder_base.h"

#include <QByteArray>

class CSGOSTUDIO_API AudioStreamEncoder : public StreamEncoderBase
{
public:
	AudioStreamEncoder(const AudioSampleDescriptor& descriptor);

protected:
	SampleDescriptorBase& GetDescriptor() override { return m_descriptor; };
	
	bool OnInitialize() override;
	int32_t GetFrameSize() const override;
	void CopyBufferToFrame() override;

	AudioSampleDescriptor m_descriptor;
};