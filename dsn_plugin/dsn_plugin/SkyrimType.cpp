#include "SkyrimType.h"

SkyrimType g_SkyrimType(SE);

UInt64 SKYRIM_VERSION[3] = {
	0x00010005003e0000,  // SkyrimSE 1.5.53.0
	0x00010004000F0000,  // SkyrimVR
	0x0000000000000000   // SkyrimVR BETA
};

std::string SKYRIM_VERSION_STR[3] = {
	"1.5.62.0",
	"1.4.15.0",
	"0.0.0.0"
};