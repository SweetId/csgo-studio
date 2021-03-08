#pragma once

#include <cstdint>

#include "teamspeak/plugin_definitions.h"
#include "teamspeak/ts3_functions.h"

static const int32_t Plugin_Api_Version = 23;

static const int32_t Error_Success = 0;
static const int32_t Error_Failure = 1;
static const int32_t Error_FailureNoPrompt = -2;

static const uint16_t CsgoStudio_ListenPort = 37015;

#define CSGOSTUDIO_VERSION_MAJOR 0
#define CSGOSTUDIO_VERSION_MINOR 1
#define CSGOSTUDIO_VERSION_REVISION 2

#define _STR(A) #A
#define STR(A) _STR(A)
#define CSGOSTUDIO_VERSION STR(CSGOSTUDIO_VERSION_MAJOR) "." STR(CSGOSTUDIO_VERSION_MINOR) "." STR(CSGOSTUDIO_VERSION_REVISION)