#pragma once

#include "qnet_connection.h"

#include <QTcpServer>
#include "qnet_client.h"

class CSGOSTUDIO_API QNetServer : public QNetConnection
{
	Q_OBJECT

public:
	QNetServer();

	virtual void Start(const QHostAddress& address, quint16 port) override;
	virtual void Stop() override;


	void Broadcast(Request& request);

signals:
	void ClientConnected(class QNetClient* client);

public slots:
	void OnNewControlConnection();
	void OnNewDataConnection();

private:
	void CreateClients();

	QTcpServer* m_controlChannel;
	QTcpServer* m_dataChannel;

	QVector<QTcpSocket*> m_pendingControlSockets;
	QVector<QTcpSocket*> m_pendingDataSockets;

	QList<QNetClient*> m_clients;
};