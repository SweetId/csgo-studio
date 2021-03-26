#include "ffmpeg.h"
#include "ffmpeg/codec_descriptor.h"

namespace ffmpeg
{
	static enum AVCodecID ToFFMpegCodec(ECodec codec)
	{
		switch (codec)
		{
		case ECodec::Mp2: return AV_CODEC_ID_MP2;
		case ECodec::None: return AV_CODEC_ID_NONE;
		default: return AV_CODEC_ID_NONE;
		}
	}

	static enum AVSampleFormat ToFFMpegFormat(ESampleFormat fmt)
	{
		switch (fmt)
		{
		case ESampleFormat::U8: return AV_SAMPLE_FMT_U8;
		case ESampleFormat::S16: return AV_SAMPLE_FMT_S16;
		case ESampleFormat::S32: return AV_SAMPLE_FMT_S32;
		case ESampleFormat::Float: return AV_SAMPLE_FMT_FLT;
		case ESampleFormat::Double: return AV_SAMPLE_FMT_DBL;
		default: return AV_SAMPLE_FMT_NONE;
		}
	}

	static ESampleFormat FromFFmpegFormat(enum AVSampleFormat fmt)
	{
		switch (fmt)
		{
		case AV_SAMPLE_FMT_U8: return ESampleFormat::U8;
		case AV_SAMPLE_FMT_S16: return ESampleFormat::S16;
		case AV_SAMPLE_FMT_S32: return ESampleFormat::S32;
		case AV_SAMPLE_FMT_FLT: return ESampleFormat::Float;
		case AV_SAMPLE_FMT_DBL: return ESampleFormat::Double;
		default: return ESampleFormat::None;
		}
	}
}