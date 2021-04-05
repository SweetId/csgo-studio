#include "video_sample_descriptor.h"
#include "private/codec_converter.h"

#include "private/ffmpeg.h"

VideoSampleDescriptor::VideoSampleDescriptor()
	: VideoSampleDescriptor(ECodec::h264, 400000, 800, 600, 30, QVideoFrame::PixelFormat::Format_YUV420P)
{}

VideoSampleDescriptor::VideoSampleDescriptor(ECodec codec, int32_t bitrate, int32_t width, int32_t height, int32_t framesPerSecond, const QVideoFrame::PixelFormat format)
	: SampleDescriptorBase(codec)
	, m_format(format)
	, m_bitRate(bitrate)
	, m_width(width)
	, m_height(height)
	, m_framesPerSecond(framesPerSecond)
{}

VideoSampleDescriptor::VideoSampleDescriptor(const VideoSampleDescriptor& o)
	: VideoSampleDescriptor(o.m_codec.GetCodec(), o.m_bitRate, o.m_width, o.m_height, o.m_framesPerSecond, o.m_format)
{
	UpdateInternal();
}

VideoSampleDescriptor& VideoSampleDescriptor::operator=(const VideoSampleDescriptor& o)
{
	m_format = o.m_format;
	m_bitRate = o.m_bitRate;
	m_width = o.m_width;
	m_height = o.m_height;
	m_framesPerSecond = o.m_framesPerSecond;
	m_codec = o.m_codec;
	UpdateInternal();
	return *this;
}

bool VideoSampleDescriptor::SetPixelFormat(QVideoFrame::PixelFormat format)
{
	m_format = format;
	return UpdateInternal();
}

bool VideoSampleDescriptor::SetBitRate(int32_t bitrate)
{
	m_bitRate = bitrate;
	return UpdateInternal();
}

bool VideoSampleDescriptor::SetCodec(ECodec codec)
{
	if (!m_codec.SetCodec(codec))
		return false;
	DeleteContext();
	return UpdateInternal();
}

bool VideoSampleDescriptor::SetWidth(int32_t width)
{
	m_width = width;
	return UpdateInternal();
}

bool VideoSampleDescriptor::SetHeight(int32_t height)
{
	m_height = height;
	return UpdateInternal();
}

bool VideoSampleDescriptor::SetFramesPerSecond(int32_t framesPerSecond)
{
	m_framesPerSecond = framesPerSecond;
	return UpdateInternal();
}

bool VideoSampleDescriptor::UpdateInternal()
{
	if (nullptr == m_privateEncodingContext)
	{
		m_privateEncodingContext = avcodec_alloc_context3(m_codec.GetEncoder());
	}
	if (nullptr == m_privateDecodingContext)
	{
		m_privateDecodingContext = avcodec_alloc_context3(m_codec.GetDecoder());
	}

	if (!IsValid())
		return false;

	m_privateEncodingContext->bit_rate = m_bitRate;
	m_privateEncodingContext->width = m_width;
	m_privateEncodingContext->height = m_height;
	m_privateEncodingContext->time_base = { 1, m_framesPerSecond };
	m_privateEncodingContext->framerate = { m_framesPerSecond, 1 };
	m_privateEncodingContext->pix_fmt = ffmpeg::ToFFMpegFormat(m_format);

	// 1 intra frame every 10 frames
	m_privateEncodingContext->gop_size = 10;
	m_privateEncodingContext->max_b_frames = 1;


	int32_t ret = avcodec_open2(m_privateEncodingContext, m_codec.GetEncoder(), nullptr);
	if (ret < 0)
	{
		DeleteContext();
		return false;
	}

	m_privateDecodingContext->bit_rate = m_bitRate;
	m_privateDecodingContext->width = m_width;
	m_privateDecodingContext->height = m_height;
	m_privateDecodingContext->time_base = { 1, m_framesPerSecond };
	m_privateDecodingContext->framerate = { m_framesPerSecond, 1 };
	m_privateDecodingContext->pix_fmt = ffmpeg::ToFFMpegFormat(m_format);

	// 1 intra frame every 10 frames
	m_privateDecodingContext->gop_size = 10;
	m_privateDecodingContext->max_b_frames = 1;

	ret = avcodec_open2(m_privateDecodingContext, m_codec.GetDecoder(), nullptr);
	if (ret < 0)
	{
		DeleteContext();
		return false;
	}

	return true;
}