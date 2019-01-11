#include "VersionCheck.h"
#include <string>
#include "Log.h"
#include <Windows.h>
#include "SkyrimType.h"

extern SkyrimType g_SkyrimType;

VersionCheck::VersionCheck()
{
}

VersionCheck::~VersionCheck()
{
}

std::string GetRuntimePath()
{
	static char	appPath[4096] = { 0 };

	if (appPath[0])
		return appPath;

	ASSERT(GetModuleFileName(GetModuleHandle(NULL), appPath, sizeof(appPath)));

	return appPath;
}

const std::string & GetRuntimeDirectory()
{
	static std::string s_runtimeDirectory;

	if (s_runtimeDirectory.empty())
	{
		std::string	runtimePath = GetRuntimePath();

		// truncate at last slash
		std::string::size_type	lastSlash = runtimePath.rfind('\\');
		if (lastSlash != std::string::npos)	// if we don't find a slash something is VERY WRONG
		{
			s_runtimeDirectory = runtimePath.substr(0, lastSlash + 1);
		}
	}

	return s_runtimeDirectory;
}


static bool GetFileVersion(const char * path, VS_FIXEDFILEINFO * info, std::string * outProductName, std::string * outProductVersion)
{
	bool result = false;

	UInt32	versionSize = GetFileVersionInfoSize(path, NULL);
	if (!versionSize)
	{
		return false;
	}

	UInt8	* versionBuf = new UInt8[versionSize];
	if (versionBuf)
	{
		if (GetFileVersionInfo(path, NULL, versionSize, versionBuf))
		{
			VS_FIXEDFILEINFO	* retrievedInfo = NULL;
			UInt32				realVersionSize = sizeof(VS_FIXEDFILEINFO);

			if (VerQueryValue(versionBuf, "\\", (void **)&retrievedInfo, (PUINT)&realVersionSize) && retrievedInfo)
			{
				*info = *retrievedInfo;
				result = true;
			}

			if (outProductName)
			{
				// try to get the product name, failure is ok
				char * productName = NULL;
				UInt32 productNameLen = 0;
				if (VerQueryValue(versionBuf, "\\StringFileInfo\\040904B0\\ProductName", (void **)&productName, (PUINT)&productNameLen) && productNameLen && productName)
				{
					*outProductName = productName;
				}
			}

			{
				char * productVersion = NULL;
				UInt32 productVersionLen = 0;
				if (VerQueryValue(versionBuf, "\\StringFileInfo\\040904B0\\ProductVersion", (void **)&productVersion, (PUINT)&productVersionLen) && productVersionLen && productVersion)
				{
					*outProductVersion = productVersion;
				}
			}
		}
		delete[] versionBuf;
	}

	return result;
}

static bool VersionStrToInt(const std::string & verStr, UInt64 * out)
{
	UInt64 result = 0;
	int parts[4];

	if (sscanf_s(verStr.c_str(), "%d.%d.%d.%d", &parts[0], &parts[1], &parts[2], &parts[3]) != 4)
		return false;

	for (int i = 0; i < 4; i++)
	{
		if (parts[i] > 0xFFFF)
			return false;

		result <<= 16;
		result |= parts[i];
	}

	*out = result;

	return true;
}

static bool GetFileVersionData(const char * path, UInt64 * out, std::string * outProductName)
{
	std::string productVersionStr;
	VS_FIXEDFILEINFO	versionInfo;
	if (!GetFileVersion(path, &versionInfo, outProductName, &productVersionStr))
		return false;

	UInt64 version = 0;
	if (!VersionStrToInt(productVersionStr, &version))
		return false;

	*out = version;

	return true;
}

std::string VersionCheck::GetSkyrimExecutableName() {
	char ExePath[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, ExePath, _countof(ExePath));
	std::string exePath(ExePath);
	size_t i = exePath.rfind('\\', exePath.length());
	return exePath.substr(i + 1, exePath.length() - i);
}

bool VersionCheck::IsCompatibleExeVersion() {
	std::string procName = VersionCheck::GetSkyrimExecutableName();

	if (procName == "SkyrimVR.exe") {
		g_SkyrimType = VR;
		
		#ifndef IS_VR
			Log::info("This dll is built for SkyrimSE and may not compatible with SkyrimVR.");
			Log::info("Please consider switching to the dll for SkyrimVR.");
		#endif
	}
	else if(procName == "SkyrimSE.exe") {
		g_SkyrimType = SE;

		#ifdef IS_VR
			Log::info("This dll is built for SkyrimVR and may not compatible with SkyrimSE.");
			Log::info("Please consider switching to the dll for SkyrimSE.");
		#endif
	}
	else {
		Log::info("Unsupported process: " + procName);
		return false;
	}

	const std::string & runtimeDir = GetRuntimeDirectory();
	std::string procPath = runtimeDir + "\\" + procName;
	UInt64 version;
	std::string	productName;
	if (!GetFileVersionData(procName.c_str(), &version, &productName))
	{
		return false;
	}

	if (version == SKYRIM_VERSION[VR_BETA])
		g_SkyrimType = VR_BETA;

	Log::info("Process name: " + procName);

	const UInt64 kSkyrimCurVersion = SKYRIM_VERSION[g_SkyrimType];

	if (version < kSkyrimCurVersion) {
		Log::info("Error: Skyrim version is out of date, please ensure you're using version " + SKYRIM_VERSION_STR[g_SkyrimType]);
		Log::hex("Skyrim Version: ", version);
		return false;
	}
	else if (version > kSkyrimCurVersion) {
		Log::info("This version of Skyrim is newer than the version supported by DSN");
		Log::info("Please install the latest version of DSN once it's available");
		Log::hex("Skyrim Version: ", version);
		return false;
	}

	Log::info("Skyrim compatibility check passed");
	Log::hex("Skyrim version: ", version);

	return true;
}
