#pragma once

#include <cstdint>

// Request sent to retrieve list of connected clients on the server
struct ListAllClientRequestHeader
{
	const static uint32_t Type = 1;
	uint8_t padding[4]; // Not used
};

// Client description
struct ClientDataHeader
{
	const static uint32_t Type = 2;

	uint16_t clientId;
	uint8_t padding[2]; // Not used
	char name[32];
};

// Sound description
struct RawSoundDataHeader
{
	const static uint32_t Type = 3;

	uint16_t clientId; // The teamspeak client id
	uint8_t padding[2]; // Not used

	uint16_t channelsCount; // Number of channels in the incoming data
	uint16_t samplesRate; // Hz sample rate (always 48000 Hz)
	uint32_t samplesCount; // Number of samples in the incoming data
	uint32_t samplesSize; // Size of the incoming data
};