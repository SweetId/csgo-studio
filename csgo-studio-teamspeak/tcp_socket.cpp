#include "tcp_socket.h"

#include <WS2tcpip.h>

#include "plugin.h"

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
		Plugin::Instance().Log(LogLevel_ERROR, "TcpSocket: socket creation failed");
		return false;
	}

	struct sockaddr_in addr = { 0 };
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = port;

	int ret = bind(m_socket, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
	if (ret == SOCKET_ERROR)
	{
		Plugin::Instance().Log(LogLevel_ERROR, "TcpSocket: socket bind failed");
		closesocket(m_socket);
		m_socket = INVALID_SOCKET;
		return false;
	}

	ret = listen(m_socket, 5);
	if (ret == SOCKET_ERROR)
	{
		Plugin::Instance().Log(LogLevel_ERROR, "TcpSocket: socket listen failed");
		closesocket(m_socket);
		m_socket = INVALID_SOCKET;
		return false;
	}

	m_address = "0.0.0.0";
	m_port = port;
	return true;
}

uint16_t TcpSocket::GetPort() const
{
	return m_port;
}

const std::string& TcpSocket::GetAddress() const
{
	return m_address;
}

int64_t TcpSocket::Recv(void* buffer, int32_t size)
{
	if (m_socket == INVALID_SOCKET)
		return 0;

	return recv(m_socket, static_cast<char*>(buffer), size, 0);
}

int64_t TcpSocket::Send(void* buffer, int32_t size)
{
	if (m_socket == INVALID_SOCKET)
		return 0;
	
	return send(m_socket, static_cast<char*>(buffer), size, 0);
}

bool TcpSocket::Accept(TcpSocket& socket, int32_t msec)
{
	fd_set accept_set;
	FD_ZERO(&accept_set);
	FD_SET(m_socket, &accept_set);

	struct timeval timeout;
	timeout.tv_usec = 1000 * msec;
	int32_t ret = select((int)m_socket + 1, &accept_set, nullptr, nullptr, &timeout);
	if (ret <= 0)
		return false;

	struct sockaddr_in addr = { 0 };
	int len = sizeof(addr);
	
	socket.m_socket = accept(m_socket, reinterpret_cast<sockaddr*>(&addr), &len);
	socket.m_port = ntohs(addr.sin_port);

	char buffer[64] = { 0 };
	if (nullptr != inet_ntop(addr.sin_family, &addr.sin_addr, buffer, sizeof(buffer)))
		socket.m_address = buffer;

	return true;
}