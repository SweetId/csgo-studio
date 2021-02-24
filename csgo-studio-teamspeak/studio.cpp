#include "studio.h"

#include "plugin.h"


void _setFunctionPointers(const struct TS3Functions funcs)
{
	Plugin::Instance().SetCallbacks(funcs);
}

int _init()
{
	return Plugin::Instance().Init();
}

void _shutdown()
{
	Plugin::Instance().Shutdown();
}

#ifdef __cplusplus
extern "C" {
#endif
	const char* ts3plugin_name()
	{
		return "CSGO-Studio";
	}

	const char* ts3plugin_description()
	{
		return "CSGO-Studio is a plugin that allows remote programs to retrieve separate audio streams over network";
	}

	const char* ts3plugin_author()
	{
		return "Sid";
	}

	const char* ts3plugin_version()
	{
		return CSGOSTUDIO_VERSION;
	}

	int ts3plugin_apiVersion()
	{
		return Plugin_Api_Version;
	}

	void ts3plugin_setFunctionPointers(const struct TS3Functions funcs)
	{
		_setFunctionPointers(funcs);
	}

	int ts3plugin_init()
	{
		return _init();
	}

	void ts3plugin_shutdown()
	{
		_shutdown();
	}

	int ts3plugin_offersConfigure()
	{
		return PLUGIN_OFFERS_NO_CONFIGURE;
	}

	void ts3plugin_onEditPostProcessVoiceDataEvent(uint64 serverConnectionHandlerID, anyID clientID, short* samples, int sampleCount, int channels, const unsigned int* channelSpeakerArray, unsigned int* channelFillMask)
	{

	}

#ifdef __cplusplus
}
#endif