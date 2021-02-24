#include "plugin.h"

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

	if (m_server.Listen(CsgoStudio_ListenPort))
	{
		Log(LogLevel_INFO, "CSGO-Studio bound on port %d", m_server.GetPort());
		return Error_Success;
	}
	else
	{
		Log(LogLevel_ERROR, "CSGO-Studio could not bind on desired port range");
		return Error_Failure;
	}
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