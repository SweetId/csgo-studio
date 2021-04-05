#pragma once

#include "common.h"
#include "video_sample_descriptor.h"
#include "stream_encoder_base.h"

#include <QByteArray>

class CSGOSTUDIO_API VideoStreamEncoder : public StreamEncoderBase
{
public:
	VideoStreamEncoder(const VideoSampleDescriptor& descriptor);

protected:
	SampleDescriptorBase& GetDescriptor() override { return m_descriptor; };

	bool OnInitialize() override;
	int32_t GetFrameSize() const override;
	void CopyBufferToFrame() override;

	VideoSampleDescriptor m_descriptor;
};