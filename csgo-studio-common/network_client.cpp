#include "network_client.h"

NetworkClient::NetworkClient()
	: m_bRunning({ false })
{

}

NetworkClient::~NetworkClient()
{
	m_bRunning = false;
	if (m_processThread.joinable())
		m_processThread.join();
}

bool NetworkClient::StartServer(uint16_t port)
{
	if (!m_control.Listen(port))
	{
		OnWarning("Failed to bind control socket\n");
		return false;
	}
	if (!m_data.Listen(port + 1))
	{
		OnWarning("Failed to bind data socket\n");
		return false;
	}

	m_bRunning = true;
	m_processThread = std::thread([this]() { RunServer(); });
	return true;
}

bool NetworkClient::StartClient(const char* address, uint16_t port)
{
	if (!m_control.Connect(address, port))
	{
		OnWarning("Failed to bind control socket\n");
		return false;
	}
	if (!m_data.Connect(address, port + 1))
	{
		OnWarning("Failed to bind data socket\n");
		return false;
	}

	m_bRunning = true;
	m_processThread = std::thread([this]() { RunClient(); });
	return true;
}

void NetworkClient::Shutdown()
{
	m_bRunning = false;
	m_processThread.join();
	m_clients.clear();
}

void NetworkClient::RunServer()
{
	while (m_bRunning)
	{
		{
			TcpSocket control;
			TcpSocket data;
			if (m_control.Accept(control, 10))
			{
				OnClientConnected(control.GetAddress(), control.GetPort());
				if (m_data.Accept(data))
				{
					OnClientConnected(data.GetAddress(), data.GetPort());

					std::unique_lock<std::mutex> lk(m_clientsMutex);
					m_clients.push_back({ std::move(control), std::move(data) });
				}
			}
		}
		{
			// Execute requests
			std::unique_lock<std::mutex> lk(m_clientsMutex);
			std::unique_lock<std::mutex> lk2(m_requestMutex);
			for (auto& request : m_requests)
			{
				for (Client& client : m_clients)
					request->Execute(client.control, client.data);
			}
			m_requests.clear();
		}

		{
			// Read requests if available
			std::unique_lock<std::mutex> lk(m_clientsMutex);
			for (Client& client : m_clients)
				ReadFromNetwork(client.control, client.data, 10);
		}
	}
}

void NetworkClient::RunClient()
{
	while (m_bRunning)
	{
		{
			// Execute requests
			std::unique_lock<std::mutex> lk(m_requestMutex);
			for (auto& request : m_requests)
			{
				request->Execute(m_control, m_data);
			}
			m_requests.clear();
		}

		ReadFromNetwork(m_control, m_data, 10);
	}
}

void NetworkClient::AddRequest(Request* request)
{
	std::unique_lock<std::mutex> lk(m_requestMutex);
	m_requests.emplace_back(request);
}

void NetworkClient::ReadFromNetwork(TcpSocket& control, TcpSocket& data, int32_t timeout)
{
	// first receive packet id
	uint32_t id;
	if(!control.Recv(id, timeout))
		return;

	switch (id)
	{
	case ListAllClientRequestHeader::Type:
	{
		ListAllClientRequestHeader header;
		if (!control.Recv(header))
			return;

		OnListAllClientRequest();
	}
		break;
	case ClientDataHeader::Type:
	{
		ClientDataHeader header;
		if (!control.Recv(header))
			return;

		OnClientInfosReceived(header.clientId, header.name);
	}
		break;
	case RawSoundDataHeader::Type:
	{
		RawSoundDataHeader header;
		if (!control.Recv(header))
			return;

		std::unique_ptr<uint8_t[]> buffer = std::make_unique<uint8_t[]>(header.samplesSize);
		if (!data.Recv(buffer.get(), header.samplesSize))
			return;

		OnSamplesReceived(header.clientId, header.samplesSize, buffer.release());
	}
		break;
	default:
		OnError("Invalid opcode received");
		break;
	}
}