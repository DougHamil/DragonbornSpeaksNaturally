#pragma once
#ifndef _SKYRIM_TYPE_H_
#define _SKYRIM_TYPE_H_

#include "common/IPrefix.h"
#include <string>

enum SkyrimType { SE = 0, VR = 1, VR_BETA = 2 };

extern SkyrimType g_SkyrimType;

extern UInt64 SKYRIM_VERSION[3];

extern std::string SKYRIM_VERSION_STR[3];

#endif
