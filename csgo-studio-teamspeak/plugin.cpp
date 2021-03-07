#include "plugin.h"

#include <chrono>
#include <fstream>
#include <WinSock2.h>

#include "tcp_data.h"

Plugin& Plugin::Instance()
{
	static Plugin plugin;
	return plugin;
}

int Plugin::Init()
{
	WSAData wsaData;
	DWORD ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (ret != 0)
	{
		Log(LogLevel_ERROR, "CSGO-Studio failed to initialize Winsock\n");
		return Error_Failure;
	}

	m_server.OnError += [this](const char* str) { Log(LogLevel_ERROR, str); };
	m_server.OnWarning += [this](const char* str) { Log(LogLevel_WARNING, str); };
	m_server.OnInfo += [this](const char* str) { Log(LogLevel_INFO, str); };
	m_server.OnClientConnected += [this](const char* address, uint16_t port) { Log(LogLevel_INFO, "Client connected from %s:%d\n", address, port); };

	m_server.OnListAllClientRequest += [this]() { ProcessListClients(); };

	if (!m_server.StartServer(CsgoStudio_ListenPort))
		return Error_Failure;

	return Error_Success;
}

void Plugin::Shutdown()
{
	m_server.Shutdown();
	WSACleanup();
}

void Plugin::LogInternal(LogLevel level, const char* str)
{
	m_functions.logMessage(str, level, "CSGO-Studio\n", 0);
	printf(str);
	OutputDebugStringA(str);
}

void WriteToFile(const ClientData& client, const std::unique_ptr<uint8_t[]>& buffer, uint32_t size)
{
	char name[64];
	sprintf_s(name, "F:/wave_%d.wav", client.id);

	std::fstream file(name, std::ios::binary | std::ios::app);
	if (file && file.is_open())
	{
		file.write((char*)buffer.get(), size);
	}
}

uint32_t ConvertToStereo(const SoundData& sound, std::unique_ptr<uint8_t[]>& buffer)
{
	uint32_t size = sound.samplesCount * 2;

	// Allocate buffer of short samples
	std::unique_ptr<short[]> samples = std::make_unique<short[]>(size);

	// if mono, duplicate soundwave
	if (sound.channels == 1)
	{
		for (uint64_t i = 0; i < sound.samplesCount; ++i)
		{
			samples[i * 2] = sound.samples[i];
			samples[i * 2 + 1] = sound.samples[i];
		}
	}
	// if stereo just copy the full buffer
	else if (sound.channels == 2)
	{
		std::memcpy(samples.get(), sound.samples, size);
	}
	else
	{
		for (uint64_t i = 0; i < sound.samplesCount; ++i)
		{
			samples[i * 2] = sound.samples[i * sound.channels];
			samples[i * 2 + 1] = sound.samples[i * sound.channels + 1];
		}
	}

	// Converting the short buffer to uint8_t buffer
	buffer.reset((uint8_t*)samples.release());

	// Size changed as we now have uint8_t instead of short
	return size * sizeof(short);
}

void Plugin::ProcessVoiceData(uint64_t serverConnectionHandlerID, const ClientData& client, const SoundData& sound)
{
	std::unique_ptr<uint8_t[]> buffer;
	uint32_t size = ConvertToStereo(sound, buffer);

#if _DEBUG
	WriteToFile(client, buffer, size);
#endif

	RawSoundDataHeader data;
	data.clientId = client.id;
	data.channelsCount = 2;
	data.samplesRate = 48000;
	data.samplesCount = sound.samplesCount;
	data.samplesSize = size;

	std::unique_ptr<TRequestWithData<RawSoundDataHeader>> request = std::make_unique<TRequestWithData<RawSoundDataHeader>>();
	request->header = data;
	request->buffer = std::move(buffer);
	request->size = size;
	m_server.AddRequest(request.release());
}

void Plugin::ProcessListClients()
{
	struct TSClient
	{
		TSClient(int32_t id, char* nameptr)
			: id(id)
		{
			strncpy_s(name, nameptr, 32);
		}

		int32_t id;
		char name[32];
	};

	std::vector<TSClient> tsClients;

	Log(LogLevel_INFO, "ProcessListClients()\n");

	for (int64_t serverId: m_serverConnectionHandleIDs)
	{
		Log(LogLevel_INFO, "ProcessListClients server %d\n", serverId);
		anyID* clients = nullptr;
		int res = m_functions.getClientList(serverId, &clients);
		if (res == Error_Success)
		{
			for (int32_t i = 0; clients[i]; ++i)
			{
				int32_t id = clients[i];
				char name[32] = { 0 };
				m_functions.getClientDisplayName(serverId, clients[i], name, 32);

				auto it = std::find_if(std::begin(tsClients), std::end(tsClients), [id](const TSClient& cl) { return cl.id == id; });
				if (it == std::end(tsClients))
				{
					tsClients.push_back({ id, name });
				}
			}
		}

		m_functions.freeMemory(clients);
	}

	for (const TSClient& client : tsClients)
	{
		std::unique_ptr<TRequest<ClientDataHeader>> request = std::make_unique<TRequest<ClientDataHeader>>();
		request->header.clientId = client.id;
		strncpy_s(request->header.name, client.name, 32);

		Log(LogLevel_INFO, "Sending client infos: %s %d\n", client.name, client.id);
		m_server.AddRequest(request.release());
	}
}

void Plugin::PrintError(int32_t errorCode)
{
	char* message = nullptr;
	if (m_functions.getErrorMessage(errorCode, &message) == Error_Success)
	{
		Log(LogLevel_ERROR, "[ERROR]: %s\n", message);
		m_functions.freeMemory(message);
	}
}

void Plugin::AddServerConnectionHandleId(uint64_t id)
{
	m_serverConnectionHandleIDs.insert(id);
}

void Plugin::RemoveServerConnectionHandleId(uint64_t id)
{
	m_serverConnectionHandleIDs.erase(id);
}