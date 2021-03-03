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

class Request
{
public:
	virtual void Execute(TcpSocket& control, TcpSocket& data) = 0;
};

template <typename THeader>
class TRequest : public Request
{
public:
	THeader header;

	virtual void Execute(TcpSocket& control, TcpSocket& data) override
	{
		control.Send(THeader::Type);
		control.Send(header);
	}
};

template <typename THeader>
class TRequestWithData : public TRequest<THeader>
{
public:
	std::unique_ptr<uint8_t[]> buffer;
	uint32_t size;

	virtual void Execute(TcpSocket& control, TcpSocket& data) override
	{
		TRequest::Execute(control, data);
		data.Send(buffer.get(), size);
	}
};

class CSGOSTUDIO_API NetworkClient
{
public:
	NetworkClient();

	bool StartServer(uint16_t port);
	bool StartClient(const char* address, uint16_t port);

	void Shutdown();

	void AddRequest(Request* request);

	Callback<const char*> OnError, OnWarning, OnInfo;
	Callback<const char*, uint16_t> OnClientConnected;

	Callback<> OnListAllClientRequest;
	Callback<uint32_t, const char*> OnClientInfosReceived;

	Callback<uint16_t, uint32_t, uint8_t*> OnSamplesReceived;

private:
	void RunServer();
	void RunClient();

	void ReadFromNetwork(TcpSocket& control, TcpSocket& data, int32_t timeout);

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

	std::mutex m_requestMutex;
	std::vector<std::unique_ptr<Request>> m_requests;
};