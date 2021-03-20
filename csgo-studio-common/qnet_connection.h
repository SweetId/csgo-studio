#pragma once

#include "common.h"

#include <QObject>
#include <QTcpSocket>

class CSGOSTUDIO_API QNetConnection : public QObject
{
	Q_OBJECT

public:
	virtual void Start(const QHostAddress& address, quint16 port) = 0;
	virtual void Stop() = 0;

signals:
	void ErrorOccured(QString message);

public slots:
	void OnErrorOccurred(QAbstractSocket::SocketError error);
};