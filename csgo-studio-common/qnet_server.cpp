#include "qnet_server.h"

QNetServer::QNetServer()
	: m_controlChannel(new QTcpServer(this))
	, m_dataChannel(new QTcpServer(this))
{
	connect(m_controlChannel, &QTcpServer::newConnection, this, &QNetServer::OnNewControlConnection);
	connect(m_controlChannel, &QTcpServer::acceptError, this, &QNetConnection::OnErrorOccurred);

	connect(m_dataChannel, &QTcpServer::newConnection, this, &QNetServer::OnNewDataConnection);
	connect(m_dataChannel, &QTcpServer::acceptError, this, &QNetConnection::OnErrorOccurred);
}

void QNetServer::Start(const QHostAddress& address, quint16 port)
{
	m_controlChannel->listen(address, port);
	m_dataChannel->listen(address, port + 1);
}

void QNetServer::Stop()
{
	m_controlChannel->close();
	m_dataChannel->close();
}

void QNetServer::Broadcast(Request& request)
{
	for (QNetClient* client : m_clients)
		client->Send(request);
}

void QNetServer::OnNewControlConnection()
{
	while (m_controlChannel->hasPendingConnections())
	{
		QTcpSocket* control = m_controlChannel->nextPendingConnection();
		m_pendingControlSockets.push_back(control);
	}

	CreateClients();
}

void QNetServer::OnNewDataConnection()
{
	while (m_dataChannel->hasPendingConnections())
	{
		QTcpSocket* data = m_dataChannel->nextPendingConnection();
		m_pendingDataSockets.push_back(data);
	}
	CreateClients();
}

void QNetServer::CreateClients()
{
	for (qint32 i = 0; i < m_pendingControlSockets.size();)
	{
		bool bFound = false;
		for (qint32 j = 0; j < m_pendingDataSockets.size();)
		{
			if (m_pendingControlSockets[i]->peerAddress() == m_pendingDataSockets[j]->peerAddress())
			{
				m_clients.push_back(new QNetClient(m_pendingControlSockets[i], m_pendingDataSockets[j]));

				m_pendingControlSockets.removeAt(i);
				m_pendingDataSockets.removeAt(i);
				bFound = true;

				emit ClientConnected(m_clients.back());
				break;
			}
			++j;
		}

		if (!bFound)
			++i;
	}
}