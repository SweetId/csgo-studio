#pragma once

#include <atomic>
#include <cstdio>
#include <mutex>
#include <thread>
#include <vector>

#include "defines.h"
#include "tcp_socket.h"

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

// Header of data sent by the plugin to server
struct OutgoingSoundDataHeader
{
	uint16_t clientId; // The teamspeak client id
	uint8_t padding[2]; // Not used

	uint16_t channelsCount; // Number of channels in the incoming data
	uint16_t samplesFrequency; // Hz sample rate (always 48000 Hz)
	uint32_t samplesCount; // Number of samples in the incoming data
	uint32_t samplesSize; // Size of the incoming data
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
	void WriteToRaw(const ClientData& client, const SoundData& sound);

	void RunServer();

	std::atomic_bool m_bRunning;
	std::thread m_processThread;

	TcpSocket m_server;
	std::vector<TcpSocket> m_clients;
	std::mutex m_clientsMutex;

	TS3Functions m_functions;
};