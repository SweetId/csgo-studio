#include "audio_stream_decoder.h"

#include "private/ffmpeg.h"

#include <QByteArray>

AudioStreamDecoder::AudioStreamDecoder(const AudioSampleDescriptor& descriptor)
	: m_descriptor(descriptor)
{
	Initialize();
}

int32_t AudioStreamDecoder::GetFrameSize() const
{
	return av_get_bytes_per_sample(m_descriptor.GetDecodingContext()->sample_fmt);
}

void AudioStreamDecoder::CopyFrameToBuffer(AVFrame* frame, QByteArray& outBuffer)
{
	const int32_t dataSize = GetFrameSize();
	const int32_t channels = m_descriptor.GetDecodingContext()->channels;

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