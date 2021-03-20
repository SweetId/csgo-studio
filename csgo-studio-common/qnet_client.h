#pragma once

#include "qnet_connection.h"

#include <QAudioBuffer>
#include <QTcpSocket>
#include <QImage>

#include <memory>
#include <queue>

class Request
{
public:
	virtual ~Request() = default;
	virtual quint32 GetType() const = 0;

	virtual void SendControl(QDataStream& stream) = 0;
	virtual void RecvControl(QDataStream& stream) = 0;

	virtual void SendData(QDataStream& stream) = 0;
	virtual void RecvData(QDataStream& stream) = 0;
};

template <typename THeader, typename TData>
class TRequest : public Request
{
public:
	THeader header;
	TData data;

	TRequest() {}

	TRequest(const THeader& header, const TData& data)
		: header(header)
		, data(data)
	{}

	virtual quint32 GetType() const override
	{
		return THeader::Type;
	}

	virtual void SendControl(QDataStream& stream) override
	{
		stream << THeader::Type;
		stream << header;
	}
	virtual void RecvControl(QDataStream& stream) override
	{
		stream >> header;
	}

	virtual void SendData(QDataStream& stream) override
	{
		stream << data;
	}
	virtual void RecvData(QDataStream& stream) override
	{
		stream >> data;
	}
};

class CSGOSTUDIO_API QNetClient : public QNetConnection
{
	Q_OBJECT

public:
	QNetClient();
	QNetClient(QTcpSocket* control, QTcpSocket* data);

	void Start(const QHostAddress& address, quint16 port) override;
	void Stop() override;
	
	bool IsConnected() const;

	void Send(Request& request);

signals:
	void ConnectedToServer();
	void DisconnectedFromServer();

	void CameraFrameReceived(QImage image);
	void MicrophoneSamplesReceived(QByteArray sound);

protected slots:
	void OnControlConnectionEstablished();
	void OnControlConnectionClosed();
	void OnControlBytesReceived();

	void InterpretControlBytes();

	void OnDataConnectionEstablished();
	void OnDataConnectionClosed();
	void OnDataBytesReceived();

	void InterpretDataBytes();

protected:
	void BindEvents();

	QTcpSocket* m_controlChannel;
	QTcpSocket* m_dataChannel;

	QByteArray m_controlBuffer;
	QByteArray m_dataBuffer;

	std::queue<std::unique_ptr<Request>> m_pendingRequests;
};