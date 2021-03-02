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

	m_server.OnError += [this](const char* str) { Log(LogLevel_ERROR, str); };
	m_server.OnWarning += [this](const char* str) { Log(LogLevel_WARNING, str); };
	m_server.OnInfo += [this](const char* str) { Log(LogLevel_INFO, str); };
	m_server.OnClientConnected += [this](const char* address, uint16_t port) { Log(LogLevel_INFO, "Client connected from %s:%d", address, port); };

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
	m_functions.logMessage(str, level, "CSGO-Studio", 0);
	printf(str);
	OutputDebugStringA(str);
}

void Plugin::WriteToFile(const ClientData& client, const std::unique_ptr<short[]>& buffer, uint32_t size)
{
	char name[64];
	sprintf_s(name, "F:/wave_%d.wav", client.id);

	std::fstream file(name, std::ios::binary | std::ios::app);
	if (file && file.is_open())
	{
		file.write((char*)buffer.get(), size * sizeof(short));
	}
	else
	{
		Log(LogLevel_INFO, "Unable to open file %s to output wave\n", name);
	}
}

uint32_t ConvertToStereo(const SoundData& sound, std::unique_ptr<short[]>& buffer)
{
	uint32_t size = sound.samplesCount * 2;
	buffer = std::make_unique<short[]>(size);

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
		std::memcpy(buffer.get(), sound.samples, size);
	}
	else
	{
		for (uint64_t i = 0; i < sound.samplesCount; ++i)
		{
			buffer[i * 2] = sound.samples[i * sound.channels];
			buffer[i * 2 + 1] = sound.samples[i * sound.channels + 1];
		}
	}

	return size;
}

void Plugin::ProcessVoiceData(uint64_t serverConnectionHandlerID, const ClientData& client, const SoundData& sound)
{
	std::unique_ptr<short[]> buffer;
	uint32_t size = ConvertToStereo(sound, buffer);

#if _DEBUG
	Log(LogLevel_INFO, "%d emited %d samples (%d channels) - data size: %d\n", client.id, sound.samplesCount, sound.channels, size);
	WriteToFile(client, buffer, size);
#endif

	OutgoingSoundDataHeader data;
	data.clientId = client.id;
	data.channelsCount = 2;
	data.samplesRate = 48000;
	data.samplesCount = sound.samplesCount;
	data.samplesSize = sizeof(short) * size;

	m_server.Broadcast(data, (uint8_t*)buffer.get(), data.samplesSize);
}