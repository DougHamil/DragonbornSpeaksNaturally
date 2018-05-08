#include "SkyrimType.h"

SkyrimType g_SkyrimType(SE);

UInt64 SKYRIM_VERSION[3] = {
	0x0001000500270000, // SkyrimSE 1.5.39.0
	0x0001000300400000,  // SkyrimVR
	0x00010004000F0000  // SkyrimVR BETA
};

std::string SKYRIM_VERSION_STR[3] = {
	"1.5.39.0",
	"1.3.64.0",
	"1.4.15.0"
};