#include "ffmpeg.h"
#include "ffmpeg/codec_descriptor.h"

#include <QVideoFrame>

namespace ffmpeg
{
	static enum AVCodecID ToFFMpegCodec(ECodec codec)
	{
		switch (codec)
		{
		case ECodec::Mp2: return AV_CODEC_ID_MP2;
		case ECodec::h264: return AV_CODEC_ID_H264;
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

	static enum AVPixelFormat ToFFMpegFormat(QVideoFrame::PixelFormat fmt)
	{
		switch (fmt)
		{
		case QVideoFrame::PixelFormat::Format_RGB24: return AV_PIX_FMT_RGB24;
		case QVideoFrame::PixelFormat::Format_BGR24: return AV_PIX_FMT_BGR24;
		case QVideoFrame::PixelFormat::Format_RGB32: return AV_PIX_FMT_RGBA;
		case QVideoFrame::PixelFormat::Format_BGRA32: return AV_PIX_FMT_BGRA;

		case QVideoFrame::PixelFormat::Format_YUYV: return AV_PIX_FMT_YUYV422;
		case QVideoFrame::PixelFormat::Format_YUV420P: return AV_PIX_FMT_YUV420P;
		default: return AV_PIX_FMT_NONE;
		}
	}

	static QVideoFrame::PixelFormat FromFFmpegFormat(enum AVPixelFormat fmt)
	{
		switch (fmt)
		{
		case AV_PIX_FMT_RGB24: return QVideoFrame::PixelFormat::Format_BGR24;
		case AV_PIX_FMT_BGR24: return QVideoFrame::PixelFormat::Format_BGR24;
		case AV_PIX_FMT_RGBA: return QVideoFrame::PixelFormat::Format_RGB32;
		case AV_PIX_FMT_BGRA: return QVideoFrame::PixelFormat::Format_BGRA32;
		case AV_PIX_FMT_YUYV422: return QVideoFrame::PixelFormat::Format_YUYV;
		case AV_PIX_FMT_YUV420P: return QVideoFrame::PixelFormat::Format_YUV420P;
		default: return QVideoFrame::PixelFormat::Format_Invalid;
		}
	}
}