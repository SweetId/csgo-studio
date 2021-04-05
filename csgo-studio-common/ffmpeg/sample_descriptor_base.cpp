#include "sample_descriptor_base.h"

#include "private/ffmpeg.h"


SampleDescriptorBase::SampleDescriptorBase(ECodec codec)
	: m_codec(codec)
	, m_privateEncodingContext(nullptr)
	, m_privateDecodingContext(nullptr)
{}

SampleDescriptorBase::~SampleDescriptorBase()
{
	DeleteContext();
}

void SampleDescriptorBase::DeleteContext()
{
	if (nullptr != m_privateEncodingContext)
		avcodec_free_context(&m_privateEncodingContext);
	m_privateEncodingContext = nullptr;

	if (nullptr != m_privateDecodingContext)
		avcodec_free_context(&m_privateDecodingContext);
	m_privateDecodingContext = nullptr;
}

bool SampleDescriptorBase::IsValid() const
{
	return m_codec.IsValid() && nullptr != m_privateEncodingContext && nullptr != m_privateDecodingContext;
}