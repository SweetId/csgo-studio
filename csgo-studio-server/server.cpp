#include "server.h"

#include "track.h"
#include "multi_track.h"
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

			// Send all datas currently recorded
			QMultiTrack* track = GetOrCreateMultiTrack(client.id);
			auto* audioTrack = track->GetAudioTrack();
			if (nullptr != audioTrack)
			{
				for (auto it = audioTrack->begin(); it != audioTrack->end(); ++it)
				{
					QNetSoundwave wave;
					wave.id = client.id;
					wave.timestamp = it->timestamp;
					wave.size = it->data.size();
					m_directorsServer.Broadcast(TRequestWithData<QNetSoundwave, QByteArray>(wave, it->data));
				}
			}
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
	// Store samples in the client track
	QMultiTrack* track = GetOrCreateMultiTrack(header.id);
	track->AddAudioSample(header.timestamp, samples);

	m_directorsServer.Broadcast(TRequestWithData<QNetSoundwave, QByteArray>(header, samples));
}

QMultiTrack* Server::GetOrCreateMultiTrack(quint32 id)
{
	auto it = m_tracks.find(id);
	if (it != m_tracks.end())
		return it.value();

	m_tracks[id] = new QMultiTrack(id, "", this);
	return m_tracks[id];
}