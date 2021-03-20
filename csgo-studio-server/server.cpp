#include "server.h"

#include "qnet_data.h"

Server::Server(quint16 clientsPort, quint16 directorPort)
	: m_nextClientId(0)
{
	m_clientsServer.Start(QHostAddress::AnyIPv4, clientsPort);
	m_directorsServer.Start(QHostAddress::AnyIPv4, directorPort);

	connect(&m_clientsServer, &QNetServer::ClientConnected, [this](QNetClient* client) {
		m_clientIds[client] = m_nextClientId++;
		connect(client, &QNetClient::CameraFrameReceived, [this, client](QImage image) { OnCameraFrameReceived(m_clientIds[client], image); });
		connect(client, &QNetClient::MicrophoneSamplesReceived, [this, client](QByteArray samples) { OnMicrophoneSampleReceived(m_clientIds[client], samples); });
	});
}

void Server::OnCameraFrameReceived(quint32 clientId, const QImage& image)
{
	QNetCameraFrame header;
	header.id = clientId;
	header.size = image.sizeInBytes();
	m_directorsServer.Broadcast(TRequest<QNetCameraFrame, QImage>(header, image));
}

void Server::OnMicrophoneSampleReceived(quint32 clientId, const QByteArray& samples)
{
	QNetSoundwave header;
	header.id = clientId;
	header.size = samples.size();
	m_directorsServer.Broadcast(TRequest<QNetSoundwave, QByteArray>(header, samples));
}