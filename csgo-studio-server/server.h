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
	void OnClientIdentifierReceived(const QNetClientIdentifier& header);
	void OnCameraFrameReceived(const QNetCameraFrame& header, const QImage& image);
	void OnMicrophoneSampleReceived(const QNetSoundwave& header, const QByteArray& samples);

protected:
	QNetServer m_clientsServer;
	QNetServer m_directorsServer;

	quint32 m_nextClientId;

	struct ClientInfo
	{
		quint32 id;
		QString name;
	};
	QMap<QNetClient*, ClientInfo> m_clientIds;
};