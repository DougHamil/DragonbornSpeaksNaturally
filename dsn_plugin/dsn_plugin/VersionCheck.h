#pragma once
#include <string>
#include "common/IPrefix.h"

class VersionCheck
{
public:
	VersionCheck();
	~VersionCheck();

	static bool IsCompatibleExeVersion();
	static std::string GetSkyrimExecutableName();
};

