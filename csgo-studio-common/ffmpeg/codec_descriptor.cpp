#include "codec_descriptor.h"
#include "private/codec_converter.h"

#include "private/ffmpeg.h"

namespace Private
{
	std::set<int32_t> ListSampleRates(AVCodec* codec)
	{
		if (nullptr == codec)
			return {};

		std::set<int32_t> rates;

		const int32_t* sample = codec->supported_samplerates;
		while (*sample)
		{
			rates.insert(*sample);
			++sample;
		}

		return rates;
	}

	std::set<int32_t> SupportedChannels(AVCodec* codec)
	{
		if (nullptr == codec)
			return {};

		std::set<int32_t> channelCounts;

		const uint64_t* layout = codec->channel_layouts;
		while (*layout)
		{
			int channels = av_get_channel_layout_nb_channels(*layout);

			channelCounts.insert(channels);
			++layout;
		}

		return channelCounts;
	}

	std::set<ESampleFormat> SupportedFormats(AVCodec* codec)
	{
		if (nullptr == codec)
			return {};

		std::set<ESampleFormat> formats;

		const enum AVSampleFormat* fmt = codec->sample_fmts;
		while (*fmt != AV_SAMPLE_FMT_NONE) {

			formats.insert(ffmpeg::FromFFmpegFormat(*fmt));
			++fmt;
		}
		return formats;
	}
}

CodecDescriptor::CodecDescriptor(ECodec codec)
	: m_codec(codec)
	, m_privateEncoder(nullptr)
	, m_privateDecoder(nullptr)
{
	SetCodec(codec);
}

CodecDescriptor::~CodecDescriptor() {}

bool CodecDescriptor::IsValid() const
{
	return nullptr != m_privateEncoder && nullptr != m_privateDecoder;
}

bool CodecDescriptor::SetCodec(ECodec codec)
{
	m_privateEncoder = avcodec_find_encoder(ffmpeg::ToFFMpegCodec(codec));
	m_privateDecoder = avcodec_find_decoder(ffmpeg::ToFFMpegCodec(codec));
	return IsValid();
}

std::set<int32_t> CodecDescriptor::SupportedEncodingSampleRates() const { return Private::ListSampleRates(m_privateEncoder); }
std::set<int32_t> CodecDescriptor::SupportedEncodingChannels() const { return Private::SupportedChannels(m_privateEncoder); }
std::set<ESampleFormat> CodecDescriptor::SupportedEncodingFormats() const { return Private::SupportedFormats(m_privateEncoder); }

std::set<int32_t> CodecDescriptor::SupportedDecodingSampleRates() const { return Private::ListSampleRates(m_privateDecoder); }
std::set<int32_t> CodecDescriptor::SupportedDecodingChannels() const { return Private::SupportedChannels(m_privateDecoder); }
std::set<ESampleFormat> CodecDescriptor::SupportedDecodingFormats() const { return Private::SupportedFormats(m_privateDecoder); }


uint64_t CodecDescriptor::GetFirstLayoutForChannels(uint64_t channels) const
{
	if (nullptr == m_privateEncoder)
		return 0;

	const uint64_t* layout = m_privateEncoder->channel_layouts;
	while (*layout)
	{
		int nb_channels = av_get_channel_layout_nb_channels(*layout);

		if (channels == nb_channels)
			return *layout;

		++layout;
	}

	return 0;
}