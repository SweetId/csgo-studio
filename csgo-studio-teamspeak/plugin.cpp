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

Plugin::Plugin()
	: m_bRunning({ false })
{ }

int Plugin::Init()
{
	WSAData wsaData;
	DWORD ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (ret != 0)
	{
		Log(LogLevel_ERROR, "CSGO-Studio failed to initialize Winsock");
		return Error_Failure;
	}

	if (!m_server.Listen(CsgoStudio_ListenPort))
	{
		Log(LogLevel_ERROR, "CSGO-Studio could not bind on desired port range");
		return Error_Failure;
	}

	m_processThread = std::thread([this]() { RunServer(); });

	Log(LogLevel_INFO, "CSGO-Studio bound on port %d", m_server.GetPort());
	return Error_Success;
}

void Plugin::Shutdown()
{
	m_bRunning = false;
	m_processThread.join();
	WSACleanup();
}

void Plugin::LogInternal(LogLevel level, const char* str)
{
	m_functions.logMessage(str, level, "CSGO-Studio", 0);
	printf(str);
	OutputDebugStringA(str);
}

void Plugin::RunServer()
{
	m_bRunning = true;

	while (m_bRunning)
	{
		TcpSocket client;
		if (m_server.Accept(client, 1000))
		{
			Log(LogLevel_INFO, "New client from %s:%d\n", client.GetAddress(), client.GetPort());

			std::unique_lock<std::mutex> lk(m_clientsMutex);
			m_clients.push_back(std::move(client));
		}
	}
}

void Plugin::WriteToFile(const ClientData& client, const std::vector<short>& buffer)
{
	char name[64];
	sprintf_s(name, "F:/wave_%d.wav", client.id);

	std::fstream file(name, std::ios::binary | std::ios::app);
	if (file && file.is_open())
	{
		file.write((char*)buffer.data(), buffer.size());
	}
	else
	{
		Log(LogLevel_INFO, "Unable to open file %s to output wave\n", name);
	}
}

void ConvertToStereo(const SoundData& sound, std::vector<short>& buffer)
{
	buffer.resize(sound.samplesCount * sizeof(float));
	
	// if mono, duplicate soundwave
	if (sound.channels == 1)
	{
		for (uint64_t i = 0; i < sound.samplesCount; ++i)
		{
			buffer[i * 2] = sound.samples[i];
			buffer[i * 2 + 1] = sound.samples[i];
		}
	}
	// if stereo just copy the full buffer
	else if (sound.channels == 2)
	{
		std::memcpy(buffer.data(), sound.samples, sizeof(short) * sound.samplesCount * 2);
	}
	else
	{
		for (uint64_t i = 0; i < sound.samplesCount; ++i)
		{
			buffer[i * 2] = sound.samples[i * sound.channels];
			buffer[i * 2 + 1] = sound.samples[i * sound.channels + 1];
		}
	}
}

void Plugin::ProcessVoiceData(uint64_t serverConnectionHandlerID, const ClientData& client, const SoundData& sound)
{
	std::vector<short> buffer;
	ConvertToStereo(sound, buffer);

#if _DEBUG
	Log(LogLevel_INFO, "%d emited %d samples (%d channels)\n", client.id, sound.samplesCount, sound.channels);
	WriteToFile(client, buffer);
#endif

	OutgoingSoundDataHeader data;
	data.clientId = client.id;
	data.channelsCount = 2;
	data.samplesRate = 48000;
	data.samplesCount = sound.samplesCount;
	data.samplesSize = data.channelsCount * data.samplesCount * sizeof(short);

	std::unique_lock<std::mutex> lk(m_clientsMutex);
	for (TcpSocket& socket : m_clients)
	{
		socket.Send(&data, sizeof(OutgoingSoundDataHeader));
		socket.Send(buffer.data(), (int32_t)buffer.size());
	}
}