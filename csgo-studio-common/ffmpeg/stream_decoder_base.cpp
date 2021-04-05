#include "stream_decoder_base.h"

#include "private/ffmpeg.h"
#include "sample_descriptor_base.h"

StreamDecoderBase::StreamDecoderBase()
	: m_parser(nullptr)
{ }

StreamDecoderBase::~StreamDecoderBase()
{
	av_parser_close(m_parser);
}

bool StreamDecoderBase::Initialize()
{
	auto& descriptor = GetDescriptor();
	if (!descriptor.IsValid())
		return false;

	m_parser = av_parser_init(GetDescriptor().GetCodec().GetDecoder()->id);
	if (nullptr == m_parser)
		return false;

	return true;
}

bool StreamDecoderBase::Decode(const QByteArray& inBuffer, QByteArray& outBuffer)
{
	AVPacket* packet = av_packet_alloc();

	m_internalBuffer.append(inBuffer);
	while (!m_internalBuffer.isEmpty())
	{
		int32_t ret = av_parser_parse2(m_parser, GetDescriptor().GetDecodingContext(), &packet->data, &packet->size, (const uint8_t*)m_internalBuffer.constData(), m_internalBuffer.size(), AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
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

bool StreamDecoderBase::DecodeInternal(AVPacket* packet, QByteArray& outBuffer)
{
	int32_t ret = avcodec_send_packet(GetDescriptor().GetDecodingContext(), packet);
	if (ret < 0)
		return false;

	AVFrame* frame = av_frame_alloc();
	while (ret >= 0)
	{
		ret = avcodec_receive_frame(GetDescriptor().GetDecodingContext(), frame);
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

		CopyFrameToBuffer(frame, outBuffer);
	}
	av_frame_free(&frame);
	return true;
}