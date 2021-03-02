#pragma once

#include <functional>
#include <mutex>
#include <vector>

#include "tcp_data.h"
#include "tcp_socket.h"

template<typename... TArgs>
class Callback
{
public:
	void operator()(TArgs... args)
	{
		for (auto& func : m_funcs)
			func(args...);
	}

	void operator+=(std::function<void(TArgs...)> callback)
	{
		m_funcs.push_back(callback);
	}

private:
	std::vector<std::function<void(TArgs...)>> m_funcs;
};

class CSGOSTUDIO_API NetworkClient
{
public:
	NetworkClient();

	bool StartServer(uint16_t port);
	bool StartClient(const char* address, uint16_t port);

	void Shutdown();

	void Broadcast(const OutgoingSoundDataHeader& header, const uint8_t* buffer, uint32_t size);

	Callback<const char*> OnError, OnWarning, OnInfo;
	Callback<const char*, uint16_t> OnClientConnected;

	Callback<uint32_t, uint32_t, uint8_t*> OnSamplesReceived;

private:
	void RunServer();
	void RunClient();

	TcpSocket m_control; // headers and messages
	TcpSocket m_data; // Data stream

	std::atomic_bool m_bRunning;
	std::thread m_processThread;

	std::mutex m_clientsMutex;
	struct Client
	{
		TcpSocket control; // headers and messages
		TcpSocket data; // Data stream
	};
	std::vector<Client> m_clients;
};