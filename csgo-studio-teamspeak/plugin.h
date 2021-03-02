#pragma once

#include <atomic>
#include <cstdio>
#include <mutex>
#include <thread>
#include <vector>

#include "defines.h"
#include "network_client.h"

// Client data sent by teamspeak API
struct ClientData
{
	anyID id;
	const unsigned int* channelSpeakerArray;
	unsigned int* channelFillMask;
};

// Sound data sent by Teamspeak API
struct SoundData
{
	const short* samples;
	int samplesCount;
	int channels;
};

struct Plugin
{
	Plugin();

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
	void WriteToFile(const ClientData& client, const std::unique_ptr<short[]>& buffer, uint32_t size);

	NetworkClient m_server;

	TS3Functions m_functions;
};