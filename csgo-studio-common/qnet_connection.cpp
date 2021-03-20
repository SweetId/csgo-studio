#include "qnet_connection.h"

void QNetConnection::OnErrorOccurred(QAbstractSocket::SocketError error)
{
	switch (error)
	{
	case QAbstractSocket::SocketError::HostNotFoundError:
		emit ErrorOccured("Host not found");
		break;
	case QAbstractSocket::SocketError::ConnectionRefusedError:
		emit ErrorOccured("Connection refused");
		break;
	}
}