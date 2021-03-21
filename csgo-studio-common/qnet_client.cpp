#include "qnet_client.h"

#include <QBuffer>
#include <QHostAddress>
#include <QImage>

#include <functional>

#define REQUESTWITHOUTDATA(Header) \
	{ Header::Type, []() { return new TRequest<Header>; } }

#define REQUESTWITHDATA(Header, Data) \
	{ Header::Type, []() { return new TRequestWithData<Header, Data>; } }

QMap<quint32, std::function<Request* ()>> RequestsFactory =
{
	REQUESTWITHOUTDATA(QNetClientIdentifier),
	REQUESTWITHDATA(QNetCameraFrame, QImage),
	REQUESTWITHDATA(QNetSoundwave, QByteArray),
	REQUESTWITHOUTDATA(QNetServerSession),
};

#define CALLBACKWITHOUTDATA(Header, Event) \
	{ Header::Type, [](QNetClient* client, Request* request) { emit client->Event(static_cast<TRequest<Header>*>(request)->header); } }

#define CALLBACKWITHDATA(Header, Data, Event) \
	{ Header::Type, [](QNetClient* client, Request* request) { emit client->Event(static_cast<TRequestWithData<Header, Data>*>(request)->header, static_cast<TRequestWithData<Header, Data>*>(request)->data); } }

QMap<quint32, std::function<void(QNetClient* , Request*)>> RequestsCallbacks =
{
	CALLBACKWITHOUTDATA(QNetClientIdentifier, ClientIdentifierReceived),
	CALLBACKWITHDATA(QNetCameraFrame, QImage, CameraFrameReceived),
	CALLBACKWITHDATA(QNetSoundwave, QByteArray, MicrophoneSamplesReceived),
	CALLBACKWITHOUTDATA(QNetServerSession, ServerSessionReceived),
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
		qint64 bytesSent = 0;
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
		QBuffer buffer(&m_controlBuffer);
		buffer.open(QIODevice::ReadOnly);
		QDataStream stream(&buffer);
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
		m_controlBuffer.remove(0, buffer.pos());

		if (request->HasData())
		{
			m_pendingRequests.push(std::move(request));

			// Try interpreting data in case we already have enough in the buffer
			InterpretDataBytes();
		}
		else
		{
			InvokeCallback(request);
		}
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

		InvokeCallback(request);

		m_pendingRequests.pop();
	}
}

void QNetClient::InvokeCallback(std::unique_ptr<Request>& request)
{
	RequestsCallbacks[request->GetType()](this, request.get());
}