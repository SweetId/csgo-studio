#pragma once

#include <string>

#include "common.h"

class CSGOSTUDIO_API TcpSocket
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
	bool Connect(const char* address, uint16_t port);

	uint16_t GetPort() const;
	const char* GetAddress() const;

	bool Recv(void* buffer, const int64_t size, const int32_t timeout = 0);
	bool Send(const void* buffer, const int64_t size);

	template<typename T>
	bool Recv(T& data, const int32_t timeout = 0) { return Recv(&data, sizeof(T), timeout); }

	template<typename T>
	int64_t Send(const T& data) { return Send(&data, sizeof(T)); }

	bool Accept(TcpSocket& socket, const int32_t timeout = 0);

private:
	void Reset();

	uint64_t m_socket;
	uint16_t m_port;
	std::string m_address;
};
