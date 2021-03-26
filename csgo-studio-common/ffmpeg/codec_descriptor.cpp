#include "codec_descriptor.h"
#include "private/codec_converter.h"

#include "private/ffmpeg.h"

CodecDescriptor::CodecDescriptor(ECodec codec)
	: m_privateCodec(nullptr)
	, m_codec(codec)
{
	SetCodec(codec);
}

CodecDescriptor::~CodecDescriptor() {}

bool CodecDescriptor::IsValid() const
{
	return nullptr != m_privateCodec;
}

bool CodecDescriptor::SetCodec(ECodec codec)
{
	m_privateCodec = avcodec_find_encoder(ffmpeg::ToFFMpegCodec(codec));
	return IsValid();
}

std::set<int32_t> CodecDescriptor::SupportedSampleRates() const
{
	if (nullptr == m_privateCodec)
		return {};

	std::set<int32_t> rates;

	const int32_t* sample = m_privateCodec->supported_samplerates;
	while (*sample)
	{
		rates.insert(*sample);
		++sample;
	}

	return rates;
}

std::set<int32_t> CodecDescriptor::SupportedChannels() const
{
	if (nullptr == m_privateCodec)
		return {};

	std::set<int32_t> channelCounts;

	const uint64_t* layout = m_privateCodec->channel_layouts;
	while (*layout)
	{
		int channels = av_get_channel_layout_nb_channels(*layout);

		channelCounts.insert(channels);
		++layout;
	}

	return channelCounts;
}

std::set<ESampleFormat> CodecDescriptor::SupportedFormats() const
{
	if (nullptr == m_privateCodec)
		return {};

	std::set<ESampleFormat> formats;

	const enum AVSampleFormat* fmt = m_privateCodec->sample_fmts;
	while (*fmt != AV_SAMPLE_FMT_NONE) {
		
		formats.insert(ffmpeg::FromFFmpegFormat(*fmt));
		++fmt;
	}
	return formats;
}

uint64_t CodecDescriptor::GetFirstLayoutForChannels(uint64_t channels) const
{
	if (nullptr == m_privateCodec)
		return 0;

	const uint64_t* layout = m_privateCodec->channel_layouts;
	while (*layout)
	{
		int nb_channels = av_get_channel_layout_nb_channels(*layout);

		if (channels == nb_channels)
			return *layout;

		++layout;
	}

	return 0;
}