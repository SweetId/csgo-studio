#pragma once

#include <string>
#include <WinSock2.h>

#include "defines.h"

class TcpSocket
{
public:
	TcpSocket();
	~TcpSocket();

	TcpSocket(TcpSocket&& o);
	TcpSocket& operator=(TcpSocket&& o);

	// Non-copyable
	TcpSocket(const TcpSocket&) = delete;
	TcpSocket& operator=(const TcpSocket&) = delete;

	bool Listen(uint16_t port);
	uint16_t GetPort() const;
	const std::string& GetAddress() const;

	int64_t Recv(void* buffer, int32_t size);
	int64_t Send(const void* buffer, int32_t size);

	bool Accept(TcpSocket& socket, int32_t timeout);

private:
	void Reset();

	SOCKET m_socket;
	uint16_t m_port;
	std::string m_address;
};
