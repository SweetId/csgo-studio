#pragma once

#include "common.h"
#include "audio_sample_descriptor.h"
#include "stream_decoder_base.h"

#include <QByteArray>

class CSGOSTUDIO_API AudioStreamDecoder : public StreamDecoderBase
{
public:
	AudioStreamDecoder(const AudioSampleDescriptor& descriptor);

protected:
	SampleDescriptorBase& GetDescriptor() override { return m_descriptor; };
	virtual int32_t GetFrameSize() const override;
	virtual void CopyFrameToBuffer(struct AVFrame* frame, QByteArray& outBuffer) override;

	AudioSampleDescriptor m_descriptor;
};