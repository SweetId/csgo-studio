#pragma once

#include <cstdio>
#include <thread>

#include "defines.h"
#include "tcp_socket.h"

struct ClientData
{
	anyID id;
	const unsigned int* channelSpeakerArray;
	unsigned int* channelFillMask;
};

struct SoundData
{
	const short* samples;
	int samplesCount;
	int channels;
};


struct Plugin
{
	static Plugin& Instance();

	int Init();
	void Shutdown();

	void SetCallbacks(const TS3Functions& functions) { m_functions = functions; }

	void ProcessVoiceData(uint64_t serverConnectionHandlerID, const ClientData& client, const SoundData& sound);

	template<typename... TArgs>
	void Log(LogLevel level, const char* format, TArgs... args)
	{
		static char buffer[1024] = { 0 };
		sprintf_s<1024>(buffer, format, args...);
		LogInternal(level, buffer);
	}

	template<>
	void Log(LogLevel level, const char* str)
	{
		LogInternal(level, str);
	}

private:
	void LogInternal(LogLevel level, const char* str);
	void WriteToRaw(const ClientData& client, const SoundData& sound);

	TcpSocket m_server;

	TS3Functions m_functions;
};