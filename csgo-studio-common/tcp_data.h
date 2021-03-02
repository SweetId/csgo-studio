#pragma once

#include <cstdint>

// Header of data sent by the plugin to server
struct OutgoingSoundDataHeader
{
	const static uint32_t Type = 3;

	uint16_t clientId; // The teamspeak client id
	uint8_t padding[2]; // Not used

	uint16_t channelsCount; // Number of channels in the incoming data
	uint16_t samplesRate; // Hz sample rate (always 48000 Hz)
	uint32_t samplesCount; // Number of samples in the incoming data
	uint32_t samplesSize; // Size of the incoming data
};