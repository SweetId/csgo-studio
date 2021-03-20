#include "track.h"

QTrack::QTrack(quint32 id, const QString& name, QObject* parent)
	: QObject(parent)
	, m_id(id)
	, m_name(name)
{}