#include "tcp_socket.h"

#include "plugin.h"

TcpSocket::TcpSocket() 
	: m_socket(INVALID_SOCKET)
	, m_port(0)
{}

TcpSocket::~TcpSocket()
{
	closesocket(m_socket);
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

	m_port = port;
	return true;
}

uint16_t TcpSocket::GetPort() const
{
	return m_port;
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