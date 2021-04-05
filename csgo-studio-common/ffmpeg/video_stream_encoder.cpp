#include "video_stream_encoder.h"

#include "private/ffmpeg.h"

VideoStreamEncoder::VideoStreamEncoder(const VideoSampleDescriptor& descriptor)
	: m_descriptor(descriptor)
{
	Initialize();
}

bool VideoStreamEncoder::OnInitialize()
{
	m_frame->format = m_descriptor.GetEncodingContext()->pix_fmt;
	m_frame->width = m_descriptor.GetEncodingContext()->width;
	m_frame->height = m_descriptor.GetEncodingContext()->height;

	return true;
}

int32_t VideoStreamEncoder::GetFrameSize() const
{
	return m_frame->linesize[0] * m_frame->linesize[1];
}

void VideoStreamEncoder::CopyBufferToFrame()
{
	uint16_t* dst = (uint16_t*)m_frame->data[0];
	uint16_t* src = (uint16_t*)m_internalBuffer.data();

	std::memcpy(dst, src, GetFrameSize());
}