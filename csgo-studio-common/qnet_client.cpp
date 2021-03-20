#include "qnet_client.h"

#include <QHostAddress>
#include <QImage>
#include "qnet_data.h"

#include <functional>

#define REQUEST(Header, Data) \
	{ Header::Type, []() { return new TRequest<Header, Data>; } }

QMap<quint32, std::function<Request* ()>> RequestsFactory =
{
	REQUEST(QNetCameraFrame, QImage),
	REQUEST(QNetSoundwave, QByteArray),
};

#define CALLBACK(Header, Data, Event) \
	{ Header::Type, [](QNetClient* client, Request* request) { emit client->Event(static_cast<TRequest<Header, Data>*>(request)->data); } }

QMap<quint32, std::function<void(QNetClient* , Request*)>> RequestsCallbacks =
{
	CALLBACK(QNetCameraFrame, QImage, CameraFrameReceived),
	CALLBACK(QNetSoundwave, QByteArray, MicrophoneSamplesReceived),
};

QNetClient::QNetClient()
	: m_controlChannel(new QTcpSocket(this))
	, m_dataChannel(new QTcpSocket(this))
{
	BindEvents();
}

QNetClient::QNetClient(QTcpSocket* control, QTcpSocket* data)
	: m_controlChannel(control)
	, m_dataChannel(data)
{
	BindEvents();
}

void QNetClient::BindEvents()
{
	connect(m_controlChannel, &QTcpSocket::connected, this, &QNetClient::OnControlConnectionEstablished);
	connect(m_controlChannel, &QTcpSocket::disconnected, this, &QNetClient::OnControlConnectionClosed);
	connect(m_controlChannel, &QTcpSocket::errorOccurred, this, &QNetConnection::OnErrorOccurred);
	connect(m_controlChannel, &QTcpSocket::readyRead, this, &QNetClient::OnControlBytesReceived);

	connect(m_dataChannel, &QTcpSocket::connected, this, &QNetClient::OnDataConnectionEstablished);
	connect(m_dataChannel, &QTcpSocket::disconnected, this, &QNetClient::OnDataConnectionClosed);
	connect(m_dataChannel, &QTcpSocket::errorOccurred, this, &QNetConnection::OnErrorOccurred);
	connect(m_dataChannel, &QTcpSocket::readyRead, this, &QNetClient::OnDataBytesReceived);
}

void QNetClient::Start(const QHostAddress& address, quint16 port)
{
	m_controlChannel->connectToHost(address, port);
}

void QNetClient::Stop()
{
	m_controlChannel->disconnectFromHost();
	m_dataChannel->disconnectFromHost();
}

bool QNetClient::IsConnected() const
{
	return m_controlChannel->state() == QAbstractSocket::SocketState::ConnectedState
		&& m_dataChannel->state() == QAbstractSocket::SocketState::ConnectedState;
}

void QNetClient::OnControlConnectionEstablished()
{
	m_dataChannel->connectToHost(m_controlChannel->peerAddress(), m_controlChannel->peerPort() + 1);
}

void QNetClient::OnControlConnectionClosed()
{
	emit DisconnectedFromServer();
}

void QNetClient::OnDataConnectionEstablished()
{
	emit ConnectedToServer();
}

void QNetClient::OnDataConnectionClosed()
{
	emit DisconnectedFromServer();
}

void QNetClient::Send(Request& request)
{
	{
		QByteArray buffer;
		QDataStream stream(&buffer, QIODevice::WriteOnly);
		request.SendControl(stream);
		m_controlChannel->write(buffer);
	}
	{
		QByteArray buffer;
		QDataStream stream(&buffer, QIODevice::WriteOnly);
		request.SendData(stream);
		quint64 bytesSent = 0;
		bytesSent += m_dataChannel->write(buffer);
		Q_ASSERT(bytesSent == buffer.size());
	}
}

void QNetClient::OnControlBytesReceived()
{
	// Read all available bytes
	while (m_controlChannel->bytesAvailable())
	{
		QByteArray data = m_controlChannel->readAll();
		m_controlBuffer.append(data);
	}

	InterpretControlBytes();
}

void QNetClient::InterpretControlBytes()
{
	// Interpret buffer as control headers
	while (!m_controlBuffer.isEmpty())
	{
		QDataStream stream(&m_controlBuffer, QIODevice::ReadOnly);
		stream.startTransaction();

		// Get type from stream
		quint32 type;
		stream >> type;

		// Create request from extracted type
		std::unique_ptr<Request> request(RequestsFactory[type]());

		// Read full request from stream
		request->RecvControl(stream);

		// Transaction will fail if there's not enough data
		if (!stream.commitTransaction())
			return;

		// only if we succeed, remove data from the incoming buffer
		//m_controlBuffer.remove(0, dataSize);
		m_pendingRequests.push(std::move(request));

		// Try interpreting data in case we already have enough in the buffer
		InterpretDataBytes();
	}
}

void QNetClient::OnDataBytesReceived()
{
	// Read all available bytes
	while (m_dataChannel->bytesAvailable())
	{
		QByteArray data = m_dataChannel->readAll();
		m_dataBuffer.append(data);
	}

	InterpretDataBytes();
}

void QNetClient::InterpretDataBytes()
{
	// Interpret buffer as control headers
	while (!m_pendingRequests.empty())
	{
		QDataStream stream(&m_dataBuffer, QIODevice::ReadOnly);
		stream.startTransaction();

		auto& request = m_pendingRequests.front();
		request->RecvData(stream);

		// Transaction will fail if there's not enough data
		if (!stream.commitTransaction())
			return;

		RequestsCallbacks[request->GetType()](this, request.get());

		m_pendingRequests.pop();
	}
}