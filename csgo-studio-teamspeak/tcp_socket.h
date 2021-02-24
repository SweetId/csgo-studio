#pragma once

#include "defines.h"

#include <WinSock2.h>

class TcpSocket
{
public:
	TcpSocket();
	~TcpSocket();

	bool Listen(uint16_t port);
	uint16_t GetPort() const;

	int64_t Recv(void* buffer, int32_t size);
	int64_t Send(void* buffer, int32_t size);

private:
	SOCKET m_socket;
	uint16_t m_port;
};
