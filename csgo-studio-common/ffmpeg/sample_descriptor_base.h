#pragma once

#include "common.h"

#include "codec_descriptor.h"

struct CSGOSTUDIO_API SampleDescriptorBase
{
public:
	SampleDescriptorBase(ECodec codec);
	virtual ~SampleDescriptorBase();

	bool IsValid() const;

	virtual bool IsAudio() const { return false; }
	virtual bool IsVideo() const { return false; }

	struct AVCodecContext* GetEncodingContext() { return m_privateEncodingContext; }
	const struct AVCodecContext* GetEncodingContext() const { return m_privateEncodingContext; }
	struct AVCodecContext* GetDecodingContext() { return m_privateDecodingContext; }
	const struct AVCodecContext* GetDecodingContext() const { return m_privateDecodingContext; }

	CodecDescriptor& GetCodec() { return m_codec; }
	const CodecDescriptor& GetCodec() const { return m_codec; }

protected:
	void DeleteContext();
	
	virtual bool UpdateInternal() = 0;

	CodecDescriptor m_codec;

	struct AVCodecContext* m_privateEncodingContext;
	struct AVCodecContext* m_privateDecodingContext;
};