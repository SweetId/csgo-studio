#pragma once

#include "qnet_connection.h"
#include "qnet_data.h"

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
	virtual bool HasData() const = 0;

	virtual void SendControl(QDataStream& stream) = 0;
	virtual void RecvControl(QDataStream& stream) = 0;

	virtual void SendData(QDataStream& stream) = 0;
	virtual void RecvData(QDataStream& stream) = 0;
};

template <typename THeader>
class TRequest : public Request
{
public:
	THeader header;

	TRequest() {}

	TRequest(const THeader& header)
		: header(header)
	{}

	virtual quint32 GetType() const override
	{
		return THeader::Type;
	}

	virtual bool HasData() const override
	{
		return THeader::HasData;
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

	virtual void SendData(QDataStream& stream) override	{}
	virtual void RecvData(QDataStream& stream) override	{}
};



template <typename THeader, typename TData>
class TRequestWithData : public TRequest<THeader>
{
public:
	TData data;

	TRequestWithData() {}

	TRequestWithData(const THeader& header, const TData& data)
		: TRequest(header)
		, data(data)
	{}

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

	void ClientIdentifierReceived(QNetClientIdentifier identifier);
	void CameraFrameReceived(QNetCameraFrame header, QImage image);
	void MicrophoneSamplesReceived(QNetSoundwave header, QByteArray sound);
	void ServerSessionReceived(QNetServerSession header);

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
	void InvokeCallback(std::unique_ptr<Request>& request);

	QTcpSocket* m_controlChannel;
	QTcpSocket* m_dataChannel;

	QByteArray m_controlBuffer;
	QByteArray m_dataBuffer;

	std::queue<std::unique_ptr<Request>> m_pendingRequests;
};