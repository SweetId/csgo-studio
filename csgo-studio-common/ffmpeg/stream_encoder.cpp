#include "stream_encoder.h"

#include "private/ffmpeg.h"

#include <QByteArray>

StreamEncoder::StreamEncoder(const AudioSampleDescriptor& descriptor)
	: m_descriptor(descriptor)
	, m_frame(av_frame_alloc())
{
	Initialize();
}

StreamEncoder::~StreamEncoder()
{
	av_frame_free(&m_frame);
}

bool StreamEncoder::Initialize()
{
	if (!m_descriptor.IsValid())
		return false;

	m_frame->nb_samples = m_descriptor.m_privateEncodingContext->frame_size;
	m_frame->format = m_descriptor.m_privateEncodingContext->sample_fmt;
	m_frame->channels = m_descriptor.m_privateEncodingContext->channels;
	m_frame->channel_layout = m_descriptor.m_privateEncodingContext->channel_layout;

	int32_t ret = av_frame_get_buffer(m_frame, 0);
	return ret >= 0;
}

bool StreamEncoder::Encode(const QByteArray& inBuffer, QByteArray& outBuffer)
{
	const uint32_t size = inBuffer.size();
	const uint8_t* data = (const uint8_t*)inBuffer.constData();

	uint32_t offset = 0;
	while (offset < size)
	{
		int32_t ret = av_frame_make_writable(m_frame);
		if (ret < 0)
			return false;

		uint8_t* ptr = (uint8_t*)m_frame->data[0];

		const uint32_t maxsize = m_frame->linesize[0];
		const uint32_t sizeToCopy = std::min((size - offset), maxsize);

		std::memcpy(ptr, data + offset, sizeToCopy);

		offset += sizeToCopy;

		if (!EncodeInternal(m_frame, outBuffer))
			return false;
	}
	return true;
}

bool StreamEncoder::EncodeInternal(AVFrame* frame, QByteArray& outBuffer)
{
	int32_t ret = avcodec_send_frame(m_descriptor.m_privateEncodingContext, frame);
	if (ret < 0)
		return false;

	AVPacket* packet = av_packet_alloc();
	while (ret >= 0)
	{
		ret = avcodec_receive_packet(m_descriptor.m_privateEncodingContext, packet);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
		{
			av_packet_free(&packet);
			return true;
		}
		else if (ret < 0)
		{
			av_packet_free(&packet);
			return false;
		}

		outBuffer.append((const char*)packet->data, packet->size);
	}
	av_packet_free(&packet);
	return true;
}