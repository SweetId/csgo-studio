#pragma once

#include "common.h"

#include "codec_descriptor.h"
#include "sample_descriptor_base.h"

struct CSGOSTUDIO_API AudioSampleDescriptor : public SampleDescriptorBase
{
public:
	AudioSampleDescriptor();
	AudioSampleDescriptor(ECodec codec, int32_t channels, int32_t bitRate, int32_t sampleRate, ESampleFormat format);

	AudioSampleDescriptor(const AudioSampleDescriptor& o);
	AudioSampleDescriptor& operator=(const AudioSampleDescriptor& o);

	bool IsAudio() const override { return true; }

	bool SetCodec(ECodec codec);
	bool SetChannels(int32_t channels);
	bool SetBitRate(int32_t rate);
	bool SetSampleRate(int32_t rate);
	bool SetSampleFormat(ESampleFormat format);

private:
	bool UpdateInternal() override;

	ESampleFormat m_format;
	int32_t m_bitRate;
	int32_t m_sampleRate;
	int32_t m_channels;
};