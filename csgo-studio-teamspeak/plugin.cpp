#include "plugin.h"

#include <fstream>

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
		Log(LogLevel_ERROR, "CSGO-Studio failed to initialize Winsock");
		return Error_Failure;
	}

	if (!m_server.Listen(CsgoStudio_ListenPort))
	{
		Log(LogLevel_ERROR, "CSGO-Studio could not bind on desired port range");
		return Error_Failure;
	}

	Log(LogLevel_INFO, "CSGO-Studio bound on port %d", m_server.GetPort());
	return Error_Success;
}

void Plugin::Shutdown()
{
	WSACleanup();
}

void Plugin::LogInternal(LogLevel level, const char* str)
{
	m_functions.logMessage(str, level, "CSGO-Studio", 0);
	printf(str);
	OutputDebugStringA(str);
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