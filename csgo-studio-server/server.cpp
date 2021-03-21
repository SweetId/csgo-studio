#include "server.h"

#include "qnet_data.h"

#include <QDateTime>

Server::Server(quint16 clientsPort, quint16 directorPort)
	: m_startingTimestamp(QDateTime::currentMSecsSinceEpoch())
	, m_nextClientId(0)
{
	m_clientsServer.Start(QHostAddress::AnyIPv4, clientsPort);
	m_directorsServer.Start(QHostAddress::AnyIPv4, directorPort);

	connect(&m_clientsServer, &QNetServer::ClientConnected, [this](QNetClient* client) {
		m_clientIds[client].id = m_nextClientId++;
		connect(client, &QNetClient::ClientIdentifierReceived, [this, client](QNetClientIdentifier header) {
			header.id = m_clientIds[client].id;
			m_clientIds[client].name = header.name;
			OnClientIdentifierReceived(header);
		});
		connect(client, &QNetClient::CameraFrameReceived, [this, client](QNetCameraFrame header, QImage image) {
			header.id = m_clientIds[client].id;
			header.timestamp -= m_startingTimestamp;
			OnCameraFrameReceived(header, image);
		});
		connect(client, &QNetClient::MicrophoneSamplesReceived, [this, client](QNetSoundwave header, QByteArray samples) {
			header.id = m_clientIds[client].id;
			header.timestamp -= m_startingTimestamp;
			OnMicrophoneSampleReceived(header, samples);
		});
	});

	connect(&m_directorsServer, &QNetServer::ClientConnected, [this](QNetClient* director) {
		{
			QNetServerSession header;
			header.timestamp = m_startingTimestamp;
			director->Send(TRequest<QNetServerSession>(header));
		}

		// Send all connected clients to director
		for (auto& client : m_clientIds)
		{
			QNetClientIdentifier header;
			header.id = client.id;
			header.name = client.name;
			director->Send(TRequest<QNetClientIdentifier>(header));
		}
	});
}

void Server::OnClientIdentifierReceived(const QNetClientIdentifier& header)
{
	qDebug() << header.name << " (" << header.id << ") connected.";
	m_directorsServer.Broadcast(TRequest<QNetClientIdentifier>(header));
}

void Server::OnCameraFrameReceived(const QNetCameraFrame& header, const QImage& image)
{
	qDebug() << image.width() << "x" << image.height();
	m_directorsServer.Broadcast(TRequestWithData<QNetCameraFrame, QImage>(header, image));
}

void Server::OnMicrophoneSampleReceived(const QNetSoundwave& header, const QByteArray& samples)
{
	qDebug() << samples.size();
	m_directorsServer.Broadcast(TRequestWithData<QNetSoundwave, QByteArray>(header, samples));
}