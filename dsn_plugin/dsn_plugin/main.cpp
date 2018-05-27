#include "Hooks.h"
#include "SpeechRecognitionClient.h"
#include "Log.h"
#include "common/IPrefix.h"
#include "skse64_common/BranchTrampoline.h"
#include "skse64_common/Relocation.h"
#include <Windows.h>
#include "VersionCheck.h"
#include "SkyrimType.h"

static const char* VERSION = "0.15";

extern std::string g_dllPath("");
extern void * g_moduleHandle = nullptr;

extern "C"	{
	BOOL WINAPI DllMain(
		HINSTANCE hinstDLL,  // handle to DLL module
		DWORD fdwReason,     // reason for calling function
		LPVOID lpReserved)  // reserved
	{
		// Get the path to the DLL file
		char DllPath[MAX_PATH] = { 0 };
		GetModuleFileName(hinstDLL, DllPath, _countof(DllPath));
		g_dllPath = std::string(DllPath);

		// Perform actions based on the reason for calling.
		switch (fdwReason)
		{
		case DLL_PROCESS_ATTACH:

			if (!VersionCheck::IsCompatibleExeVersion()) {
				return TRUE;
			}

			// Initialize once for each new process.
			// Return FALSE to fail DLL load.
			Log::info("DragonBornNaturallySpeaking loaded");
			Log::info(std::string("Version ").append(VERSION));
			g_moduleHandle = (void *)hinstDLL;
			g_branchTrampoline.Create(1024 * 64);
			g_localTrampoline.Create(1024 * 64, g_moduleHandle);
			Hooks_Inject();

			SpeechRecognitionClient::Initialize();

			break;

		case DLL_THREAD_ATTACH:
			// Do thread-specific initialization.
			break;

		case DLL_THREAD_DETACH:
			// Do thread-specific cleanup.
			break;

		case DLL_PROCESS_DETACH:
			// Perform any necessary cleanup.
			break;
		}
		return TRUE;  // Successful DLL_PROCESS_ATTACH.
	}
};