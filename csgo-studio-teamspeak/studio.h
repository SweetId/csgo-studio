#pragma once

#include"common.h"
#include "defines.h"

#ifdef __cplusplus
extern "C" {
#endif
	/* Required functions */
	CSGOSTUDIO_API const char* ts3plugin_name();
	CSGOSTUDIO_API const char* ts3plugin_version();
	CSGOSTUDIO_API int ts3plugin_apiVersion();
	CSGOSTUDIO_API const char* ts3plugin_author();
	CSGOSTUDIO_API const char* ts3plugin_description();
	CSGOSTUDIO_API void ts3plugin_setFunctionPointers(const struct TS3Functions funcs);
	CSGOSTUDIO_API int ts3plugin_init();
	CSGOSTUDIO_API void ts3plugin_shutdown();

	/* Optional functions */
	CSGOSTUDIO_API int ts3plugin_offersConfigure();

	/**
	* @brief called before audio data is mixed together into a single audio stream for playback, but after effects (3D positioning for example) have been applied.
	*
	* @param serverConnectionHandlerID specifies on which connection the callback was called
	* @param clientID id of the source client for the audio
	* @param samples buffer of audio data for the client as 16 bit signed at 48kHz
	* @param sampleCount how many audio frames are available in the buffer
	* @param channels number of audio channels in the audio data
	* @param channelSpeakerArray Array with an entry for each channel in the buffer, defining the speaker each channel represents. see SPEAKER_* defines in public_definitions.h
	* @param channelFillMask a bit mask of SPEAKER_* that defines which of the channels in the buffer have audio data. Be sure to set the corresponding flag when adding audio to previously empty channels in the buffer.
   */
	CSGOSTUDIO_API void ts3plugin_onEditPostProcessVoiceDataEvent(uint64 serverConnectionHandlerID, anyID clientID, short* samples, int sampleCount, int channels, const unsigned int* channelSpeakerArray, unsigned int* channelFillMask);
#ifdef __cplusplus

	/**
	 * @brief called when the status of a connection changes
	 *
	 * @param serverConnectionHandlerID specifies on which connection the status has changed
	 * @param newStatus the current status of the connection. One of the values from the ConnectStatus enum
	 * @param errorNumber if the state change was caused by an error this is set to one of the values from the @ref Ts3ErrorType enum
	*/
	CSGOSTUDIO_API void ts3plugin_onConnectStatusChangeEvent(uint64 serverConnectionHandlerID, int newStatus, unsigned int errorNumber);

	/**
	 * @brief called when a client moves to a different channel, disconnects, connects, gets kicked or banned.
	 *
	 * @param serverConnectionHandlerID specifies on which connection the callback was called
	 * @param clientID id of the client changing channels
	 * @param oldChannelID id of the previous channel of the client.
	 * @param newChannelID id of the current channel of the client. Can be 0, if the client disconnected / got kicked / banned.
	*/
	CSGOSTUDIO_API void ts3plugin_onClientMoveEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, const char* moveMessage);
}
#endif