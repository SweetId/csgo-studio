#include "network_client.h"

NetworkClient::NetworkClient()
	: m_bRunning({ false })
{

}

bool NetworkClient::StartServer(uint16_t port)
{
	if (!m_control.Listen(port))
	{
		OnWarning("Failed to bind control socket");
		return false;
	}
	if (!m_data.Listen(port + 1))
	{
		OnWarning("Failed to bind data socket");
		return false;
	}

	m_processThread = std::thread([this]() { RunServer(); });
	return true;
}

bool NetworkClient::StartClient(const char* address, uint16_t port)
{
	if (!m_control.Connect(address, port))
	{
		OnWarning("Failed to bind control socket");
		return false;
	}
	if (!m_data.Connect(address, port + 1))
	{
		OnWarning("Failed to bind data socket");
		return false;
	}

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
	m_bRunning = true;

	while (m_bRunning)
	{
		TcpSocket control;
		TcpSocket data;
		if (m_control.Accept(control, 1000))
		{
			OnClientConnected(control.GetAddress(), control.GetPort());
			if (m_data.Accept(data, 1000))
			{
				OnClientConnected(data.GetAddress(), data.GetPort());

				std::unique_lock<std::mutex> lk(m_clientsMutex);
				m_clients.push_back({ std::move(control), std::move(data) });
			}
		}
	}
}

void NetworkClient::RunClient()
{
	m_bRunning = true;

	while (m_bRunning)
	{
		// first receive packet id
		uint32_t id;
		int64_t ret = m_control.Recv(id);
		if (ret != sizeof(id))
		{
			OnError("Recv error");
			break;
		}

		switch (id)
		{
		case OutgoingSoundDataHeader::Type:
		{
			OutgoingSoundDataHeader header;
			ret = m_control.Recv(header);
			if (ret != sizeof(header))
			{
				OnError("Recv error");
				return;
			}

			std::unique_ptr<uint8_t[]> data = std::make_unique<uint8_t[]>(header.samplesSize);
			ret = m_data.Recv(data.get(), header.samplesSize);
			if (ret != header.samplesSize)
			{
				OnError("Recv error");
				return;
			}

			OnSamplesReceived(header.clientId, header.samplesSize, data.release());
		}
		}
	}
}

void NetworkClient::Broadcast(const OutgoingSoundDataHeader& header, const uint8_t* buffer, uint32_t size)
{
	std::unique_lock<std::mutex> lk(m_clientsMutex);
	for (Client& client : m_clients)
	{
		client.control.Send(OutgoingSoundDataHeader::Type);
		client.control.Send(header);
		client.data.Send(buffer, size);
	}
}