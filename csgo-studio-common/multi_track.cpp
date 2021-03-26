#include "multi_track.h"

QMultiTrack::QMultiTrack(quint32 id, const QString& name, QObject* parent)
	: QObject(parent)
	, m_id(id)
	, m_name(name)
	, m_audioTrack(new Track<QByteArray>())
	//, m_videoTrack(nullptr)
{}

QMultiTrack::~QMultiTrack() {}

void QMultiTrack::AddAudioSample(quint64 timestamp, const QByteArray& samples)
{
	m_audioTrack->Add(timestamp, samples);
}