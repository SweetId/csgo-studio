#include "audio_sample_descriptor.h"
#include "private/codec_converter.h"

#include "private/ffmpeg.h"

AudioSampleDescriptor::AudioSampleDescriptor()
	: AudioSampleDescriptor(ECodec::Mp2, 2, 64000, 44100, ESampleFormat::S16)
{}

AudioSampleDescriptor::AudioSampleDescriptor(const ECodec codec, const int32_t channels, const int32_t bitRate, const int32_t sampleRate, const ESampleFormat format)
	: m_format(format)
	, m_bitRate(bitRate)
	, m_sampleRate(sampleRate)
	, m_channels(channels)
	, m_codec(codec)
	, m_privateEncodingContext(nullptr)
	, m_privateDecodingContext(nullptr)
{}

AudioSampleDescriptor::~AudioSampleDescriptor()
{
	DeleteContext();
}

AudioSampleDescriptor::AudioSampleDescriptor(const AudioSampleDescriptor& o)
	: AudioSampleDescriptor(o.m_codec.GetCodec(), o.m_channels, o.m_bitRate, o.m_sampleRate, o.m_format)
{
	UpdateInternal();
}

AudioSampleDescriptor& AudioSampleDescriptor::operator=(const AudioSampleDescriptor& o)
{
	m_format = o.m_format;
	m_bitRate = o.m_bitRate;
	m_sampleRate = o.m_sampleRate;
	m_channels = o.m_channels;
	m_codec = o.m_codec;
	UpdateInternal();
	return *this;
}

bool AudioSampleDescriptor::IsValid() const
{
	return m_codec.IsValid() && nullptr != m_privateEncodingContext && nullptr != m_privateDecodingContext;
}

bool AudioSampleDescriptor::SetCodec(ECodec codec)
{
	if (!m_codec.SetCodec(codec))
		return false;
	DeleteContext();
	return UpdateInternal();
}

bool AudioSampleDescriptor::SetBitRate(int32_t rate)
{
	m_bitRate = rate;
	return UpdateInternal();
}

bool AudioSampleDescriptor::SetChannels(int32_t channels)
{
	m_channels = channels;
	return UpdateInternal();
}

bool AudioSampleDescriptor::SetSampleRate(int32_t rate)
{
	m_sampleRate = rate;
	return UpdateInternal();
}

bool AudioSampleDescriptor::SetSampleFormat(ESampleFormat format)
{
	m_format = format;
	return UpdateInternal();
}

bool AudioSampleDescriptor::UpdateInternal()
{
	if (nullptr == m_privateEncodingContext)
	{
		m_privateEncodingContext = avcodec_alloc_context3(m_codec.m_privateEncoder);
	}
	if (nullptr == m_privateDecodingContext)
	{
		m_privateDecodingContext = avcodec_alloc_context3(m_codec.m_privateDecoder);
	}

	if (!IsValid())
		return false;

	m_privateEncodingContext->bit_rate = m_bitRate;
	m_privateEncodingContext->sample_rate = m_sampleRate;
	m_privateEncodingContext->sample_fmt = ffmpeg::ToFFMpegFormat(m_format);
	m_privateEncodingContext->channels = m_channels;
	m_privateEncodingContext->channel_layout = m_codec.GetFirstLayoutForChannels(m_channels);

	int32_t ret = avcodec_open2(m_privateEncodingContext, m_codec.m_privateEncoder, nullptr);
	if (ret < 0)
	{
		DeleteContext();
		return false;
	}

	m_privateDecodingContext->bit_rate = m_bitRate;
	m_privateDecodingContext->sample_rate = m_sampleRate;
	m_privateDecodingContext->sample_fmt = ffmpeg::ToFFMpegFormat(m_format);
	m_privateDecodingContext->channels = m_channels;
	m_privateDecodingContext->channel_layout = m_codec.GetFirstLayoutForChannels(m_channels);

	ret = avcodec_open2(m_privateDecodingContext, m_codec.m_privateDecoder, nullptr);
	if (ret < 0)
	{
		DeleteContext();
		return false;
	}

	return true;
}

void AudioSampleDescriptor::DeleteContext()
{
	if (nullptr != m_privateEncodingContext)
		avcodec_free_context(&m_privateEncodingContext);
	m_privateEncodingContext = nullptr;

	if (nullptr != m_privateDecodingContext)
		avcodec_free_context(&m_privateDecodingContext);
	m_privateDecodingContext = nullptr;
}