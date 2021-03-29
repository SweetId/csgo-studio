#include "stream_decoder.h"

#include "private/ffmpeg.h"

#include <QByteArray>

StreamDecoder::StreamDecoder(const AudioSampleDescriptor& descriptor)
	: m_descriptor(descriptor)
	, m_parser(nullptr)
{
	Initialize();
}

StreamDecoder::~StreamDecoder()
{
	av_parser_close(m_parser);
}

bool StreamDecoder::Initialize()
{
	if (!m_descriptor.IsValid())
		return false;

	m_parser = av_parser_init(m_descriptor.m_codec.m_privateDecoder->id);
	if (nullptr == m_parser)
		return false;

	return true;
}

bool StreamDecoder::Decode(const QByteArray& inBuffer, QByteArray& outBuffer)
{
	AVPacket* packet = av_packet_alloc();

	m_internalBuffer.append(inBuffer);
	while (!m_internalBuffer.isEmpty())
	{
		int32_t ret = av_parser_parse2(m_parser, m_descriptor.m_privateDecodingContext, &packet->data, &packet->size, (const uint8_t*)m_internalBuffer.constData(), m_internalBuffer.size(), AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
		if (ret < 0)
		{
			av_packet_free(&packet);
			return false;
		}

		if (!DecodeInternal(packet, outBuffer))
			return false;

		m_internalBuffer.remove(0, ret);
	}

	av_packet_free(&packet);
	return true;
}

bool StreamDecoder::DecodeInternal(AVPacket* packet, QByteArray& outBuffer)
{
	const int32_t dataSize = av_get_bytes_per_sample(m_descriptor.m_privateDecodingContext->sample_fmt);
	if (dataSize < 0)
		return false;

	const int32_t channels = m_descriptor.m_privateDecodingContext->channels;

	int32_t ret = avcodec_send_packet(m_descriptor.m_privateDecodingContext, packet);
	if (ret < 0)
		return false;

	AVFrame* frame = av_frame_alloc();
	while (ret >= 0)
	{
		ret = avcodec_receive_frame(m_descriptor.m_privateDecodingContext, frame);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
		{
			av_frame_free(&frame);
			return true;
		}
		else if (ret < 0)
		{
			av_frame_free(&frame);
			return false;
		}

		// Recreate interleaved buffer
		for (int32_t i = 0; i < frame->nb_samples; ++i)
		{
			for (int32_t j = 0; j < channels; ++j)
			{
				switch (dataSize)
				{
				case 1:
				{
					uint8_t* ptr = (uint8_t*)frame->data[j] + i;
					outBuffer.append((const char*)ptr, sizeof(uint8_t));
				}
					break;
				case 2:
				{
					uint16_t* ptr = (uint16_t*)frame->data[j] + i;
					outBuffer.append((const char*)ptr, sizeof(uint16_t));
				}
					break;
				default:
					// not handled
					break;

				}
			}
		}
	}
	av_frame_free(&frame);
	return true;
}