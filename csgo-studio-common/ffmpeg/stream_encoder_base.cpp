#include "stream_encoder_base.h"

#include "private/ffmpeg.h"
#include "sample_descriptor_base.h"

StreamEncoderBase::StreamEncoderBase()
	: m_frame(av_frame_alloc())
{
}

StreamEncoderBase::~StreamEncoderBase()
{
	av_frame_free(&m_frame);
}

bool StreamEncoderBase::Initialize()
{
	if (!OnInitialize())
		return false;

	int32_t ret = av_frame_get_buffer(m_frame, 0);
	return ret >= 0;
}

bool StreamEncoderBase::Encode(const QByteArray& inBuffer, QByteArray& outBuffer)
{
	m_internalBuffer.append(inBuffer);
	while (!m_internalBuffer.isEmpty())
	{
		int32_t ret = av_frame_make_writable(m_frame);
		if (ret < 0)
		{
			return false;
		}

		const int maxsize = GetFrameSize();

		// Not enough data in internal buffer, skip and wait for next data
		if (m_internalBuffer.size() < maxsize)
			break;

		CopyBufferToFrame();

		if (!EncodeInternal(m_frame, outBuffer))
		{
			return false;
		}

		m_internalBuffer.remove(0, maxsize);
	}
	return true;
}

bool StreamEncoderBase::EncodeInternal(AVFrame* frame, QByteArray& outBuffer)
{
	int32_t ret = avcodec_send_frame(GetDescriptor().GetEncodingContext(), frame);
	if (ret < 0)
		return false;

	AVPacket* packet = av_packet_alloc();
	while (ret >= 0)
	{
		ret = avcodec_receive_packet(GetDescriptor().GetEncodingContext(), packet);
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