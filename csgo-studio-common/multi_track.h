#pragma once

#include <QByteArray>
#include <QObject>

#include "track.h"

class QMultiTrack : public QObject
{
	Q_OBJECT

public:
	QMultiTrack(quint32 id, const QString& name, QObject* parent = nullptr);
	~QMultiTrack();

	quint32 GetId() const { return m_id; }
	
	const QString& GetName() const { return m_name; }
	void SetName(const QString& name) { m_name = name; }

	void AddAudioSample(quint64 timestamp, const QByteArray& samples);

	Track<QByteArray>* GetAudioTrack() { return m_audioTrack.get(); }

private:
	quint32 m_id;
	QString m_name;

	QScopedPointer<Track<QByteArray>> m_audioTrack;
	//class QTrack* m_videoTrack;
};