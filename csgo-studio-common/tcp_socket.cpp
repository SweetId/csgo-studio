#include "tcp_socket.h"

#include <WinSock2.h>
#include <WS2tcpip.h>

TcpSocket::TcpSocket()
{
	Reset();
}

TcpSocket::~TcpSocket()
{
	closesocket(m_socket);
}

TcpSocket::TcpSocket(TcpSocket&& o)
	: m_socket(o.m_socket)
	, m_port(o.m_port)
	, m_address(o.m_address)
{
	o.Reset();
}

TcpSocket& TcpSocket::operator=(TcpSocket&& o)
{
	m_socket = o.m_socket;
	m_port = o.m_port;
	m_address = o.m_address;
	o.Reset();
	return *this;
}

void TcpSocket::Reset()
{
	m_socket = INVALID_SOCKET;
	m_port = 0;
	m_address = "noaddr";
}

bool TcpSocket::Listen(uint16_t port)
{
	m_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (m_socket == INVALID_SOCKET)
	{
		return false;
	}

	struct sockaddr_in addr = { 0 };
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);

	int ret = bind(m_socket, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
	if (ret == SOCKET_ERROR)
	{
		closesocket(m_socket);
		m_socket = INVALID_SOCKET;
		return false;
	}

	ret = listen(m_socket, 5);
	if (ret == SOCKET_ERROR)
	{
		closesocket(m_socket);
		m_socket = INVALID_SOCKET;
		return false;
	}

	m_address = "0.0.0.0";
	m_port = port;
	return true;
}

bool TcpSocket::Connect(const char* address, uint16_t port)
{
	m_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (m_socket == INVALID_SOCKET)
		return false;

	struct sockaddr_in addr = { 0 };
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	if (inet_pton(AF_INET, address, &addr.sin_addr) != 1)
		return false;

	int ret = connect(m_socket, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
	if (ret == -1)
	{
		int err = WSAGetLastError();
		return false;
	}

	m_address = address;
	m_port = port;
	return true;
}

uint16_t TcpSocket::GetPort() const
{
	return m_port;
}

const char* TcpSocket::GetAddress() const
{
	return m_address.c_str();
}

bool TcpSocket::Recv(void* buffer, const int64_t size, const int32_t msec)
{
	if (m_socket == INVALID_SOCKET)
		return false;

	fd_set read_set;
	FD_ZERO(&read_set);
	FD_SET(m_socket, &read_set);
	
	if (msec > 0)
	{
		struct timeval timeout;
		timeout.tv_usec = 1000 * msec;
		int32_t ret = select((int)m_socket + 1, &read_set, nullptr, nullptr, &timeout);
		if (ret <= 0)
			return false;
	}

	int32_t bytes = recv(m_socket, static_cast<char*>(buffer), (int)size, 0);
	return bytes == size;
}

bool TcpSocket::Send(const void* buffer, const int64_t size)
{
	if (m_socket == INVALID_SOCKET)
		return false;
	
	int32_t bytes = send(m_socket, static_cast<const char*>(buffer), (int)size, 0);
	return bytes == size;
}

bool TcpSocket::Accept(TcpSocket& socket, const int32_t msec)
{
	fd_set accept_set;
	FD_ZERO(&accept_set);
	FD_SET(m_socket, &accept_set);

	if (msec > 0)
	{
		struct timeval timeout;
		timeout.tv_usec = 1000 * msec;
		int32_t ret = select((int)m_socket + 1, &accept_set, nullptr, nullptr, &timeout);
		if (ret <= 0)
			return false;
	}

	struct sockaddr_in addr = { 0 };
	int len = sizeof(addr);
	
	socket.m_socket = accept(m_socket, reinterpret_cast<sockaddr*>(&addr), &len);
	socket.m_port = ntohs(addr.sin_port);

	char buffer[64] = { 0 };
	if (nullptr != inet_ntop(addr.sin_family, &addr.sin_addr, buffer, sizeof(buffer)))
		socket.m_address = buffer;

	return true;
}