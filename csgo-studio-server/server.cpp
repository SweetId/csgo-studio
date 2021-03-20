#include "server.h"

#include <QDateTime>

#include "qnet_data.h"

Server::Server(quint16 clientsPort, quint16 directorPort)
	: m_nextClientId(0)
{
	m_clientsServer.Start(QHostAddress::AnyIPv4, clientsPort);
	m_directorsServer.Start(QHostAddress::AnyIPv4, directorPort);

	connect(&m_clientsServer, &QNetServer::ClientConnected, [this](QNetClient* client) {
		m_clientIds[client].id = m_nextClientId++;
		connect(client, &QNetClient::ClientIdentifierReceived, [this, client](QNetClientIdentifier identifier) {
			m_clientIds[client].name = identifier.name;
		});
		connect(client, &QNetClient::CameraFrameReceived, [this, client](QNetCameraFrame header, QImage image) {
			header.id = m_clientIds[client].id;
			OnCameraFrameReceived(header, image);
		});
		connect(client, &QNetClient::MicrophoneSamplesReceived, [this, client](QNetSoundwave header, QByteArray samples) {
			header.id = m_clientIds[client].id;
			OnMicrophoneSampleReceived(header, samples);
		});
	});
}

void Server::OnCameraFrameReceived(const QNetCameraFrame& header, const QImage& image)
{
	m_directorsServer.Broadcast(TRequestWithData<QNetCameraFrame, QImage>(header, image));
}

void Server::OnMicrophoneSampleReceived(const QNetSoundwave& header, const QByteArray& samples)
{
	m_directorsServer.Broadcast(TRequestWithData<QNetSoundwave, QByteArray>(header, samples));
}