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
	, m_privateContext(nullptr)
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
	return m_codec.IsValid() && nullptr != m_privateContext;
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
	if (nullptr == m_privateContext)
	{
		m_privateContext = avcodec_alloc_context3(m_codec.m_privateCodec);
	}

	if (!IsValid())
		return false;

	m_privateContext->bit_rate = m_bitRate;
	m_privateContext->sample_rate = m_sampleRate;
	m_privateContext->sample_fmt = ffmpeg::ToFFMpegFormat(m_format);
	m_privateContext->channels = m_channels;
	m_privateContext->channel_layout = m_codec.GetFirstLayoutForChannels(m_channels);

	int32_t ret = avcodec_open2(m_privateContext, m_codec.m_privateCodec, nullptr);
	if (ret < 0)
	{
		DeleteContext();
		return false;
	}

	return true;
}

void AudioSampleDescriptor::DeleteContext()
{
	if (nullptr != m_privateContext)
		avcodec_free_context(&m_privateContext);
	m_privateContext = nullptr;
}