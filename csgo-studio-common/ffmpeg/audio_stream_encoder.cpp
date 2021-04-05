#include "audio_stream_encoder.h"

#include "private/ffmpeg.h"

AudioStreamEncoder::AudioStreamEncoder(const AudioSampleDescriptor& descriptor)
	: m_descriptor(descriptor)
{
	Initialize();
}

bool AudioStreamEncoder::OnInitialize()
{
	m_frame->format = m_descriptor.GetEncodingContext()->sample_fmt;
	m_frame->nb_samples = m_descriptor.GetEncodingContext()->frame_size;
	m_frame->channels = m_descriptor.GetEncodingContext()->channels;
	m_frame->channel_layout = m_descriptor.GetEncodingContext()->channel_layout;

	return true;
}

int32_t AudioStreamEncoder::GetFrameSize() const
{
	return m_frame->linesize[0];
}

void AudioStreamEncoder::CopyBufferToFrame()
{
	uint16_t* dst = (uint16_t*)m_frame->data[0];
	uint16_t* src = (uint16_t*)m_internalBuffer.data();

	std::memcpy(dst, src, GetFrameSize());
}