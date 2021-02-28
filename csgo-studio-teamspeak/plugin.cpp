#include "plugin.h"

#include <chrono>
#include <fstream>

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
			Log(LogLevel_INFO, "New client from %s:%d\n", client.GetAddress().c_str(), client.GetPort());
			m_clients.push_back(std::move(client));
		}
	}
}

void Plugin::WriteToRaw(const ClientData& client, const SoundData& sound)
{
	Log(LogLevel_INFO, "%d emited %d samples (%d channels)\n", client.id, sound.samplesCount, sound.channels);
	char name[64];
	sprintf_s(name, "F:/wave_%d.wav", client.id);

	std::fstream file(name, std::ios::binary | std::ios::app);
	if (file && file.is_open())
	{
		file.write((char*)sound.samples, sizeof(short) * sound.samplesCount * sound.channels);
	}
	else
	{
		Log(LogLevel_INFO, "Unable to open file %s to output wave\n", name);
	}
}

void Plugin::ProcessVoiceData(uint64_t serverConnectionHandlerID, const ClientData& client, const SoundData& sound)
{
	WriteToRaw(client, sound);
}