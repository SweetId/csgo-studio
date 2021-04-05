#pragma once

#include "common.h"

#include "enums.h"
#include "codec_descriptor.h"
#include "sample_descriptor_base.h"

#include <QVideoFrame>

struct CSGOSTUDIO_API VideoSampleDescriptor : public SampleDescriptorBase
{
public:
	VideoSampleDescriptor();
	VideoSampleDescriptor(ECodec codec, int32_t bitrate, int32_t width, int32_t height, int32_t framesPerSecond, const QVideoFrame::PixelFormat format);

	VideoSampleDescriptor(const VideoSampleDescriptor& o);
	VideoSampleDescriptor& operator=(const VideoSampleDescriptor& o);

	bool IsVideo() const override { return true; }

	bool SetPixelFormat(QVideoFrame::PixelFormat format);
	bool SetBitRate(int32_t bitrate);
	bool SetCodec(ECodec codec);
	bool SetWidth(int32_t width);
	bool SetHeight(int32_t height);
	bool SetFramesPerSecond(int32_t framesPerSecond);

private:
	bool UpdateInternal() override;

	QVideoFrame::PixelFormat m_format;
	int32_t m_bitRate;
	int32_t m_width;
	int32_t m_height;
	int32_t m_framesPerSecond;
};