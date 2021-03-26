#include "audio_stream.h"

#include "private/ffmpeg.h"

AudioStream::AudioStream(const AudioSampleDescriptor& descriptor)
	: m_descriptor(descriptor)
	, m_frame(av_frame_alloc())
{
	Initialize();
}

AudioStream::~AudioStream()
{
	av_frame_free(&m_frame);
}

bool AudioStream::Initialize()
{
	if (!m_descriptor.IsValid())
		return false;

	m_frame->nb_samples = m_descriptor.m_privateContext->frame_size;
	m_frame->format = m_descriptor.m_privateContext->sample_fmt;
	m_frame->channel_layout = m_descriptor.m_privateContext->channel_layout;

	int32_t ret = av_frame_get_buffer(m_frame, 0);
	return ret >= 0;
}

bool AudioStream::Encode(const uint32_t size, const uint8_t* data)
{
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

		if (!EncodeInternal())
			return false;
	}
	return true;
}

bool AudioStream::EncodeInternal()
{
	int32_t ret = avcodec_send_frame(m_descriptor.m_privateContext, m_frame);
	if (ret < 0)
		return false;

	AVPacket* packet = av_packet_alloc();
	while (ret >= 0)
	{
		ret = avcodec_receive_packet(m_descriptor.m_privateContext, packet);
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

		EncodedDataAvailable(packet->size, packet->data);
	}
	av_packet_free(&packet);
	return true;
}