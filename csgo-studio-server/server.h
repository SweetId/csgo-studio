#pragma once

#include <QImage>
#include <QObject>

#include "qnet_server.h"

class Server : public QObject
{
	Q_OBJECT
public:
	Server(quint16 clientsPort, quint16 directorPort);

protected slots:
	void OnCameraFrameReceived(quint32 clientId, const QImage& image);
	void OnMicrophoneSampleReceived(quint32 clientId, const QByteArray& samples);

protected:
	QNetServer m_clientsServer;
	QNetServer m_directorsServer;

	quint32 m_nextClientId;
	QMap<QNetClient*, quint32> m_clientIds;
};