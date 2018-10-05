#pragma once
#include "common/IPrefix.h"
#include "skse64/GameMenus.h"
#include <string>
#include <vector>
#include <mutex>
#include <queue>
#include <unordered_map>
#include <functional>
#include <Windows.h>

class ConsoleCommandRunner
{
private:
	static const size_t kMaxCustomCmdQueueSize = 1000;
	
	static std::unordered_map<std::string/* name */, std::function<void(std::vector<std::string>)>/* func */> customCmdList;
	static std::mutex customCmdQueueLock;
	static HANDLE customCmdQueueSemaphore;
	static std::queue<std::vector<std::string>> customCmdQueue;

public:
	// Run a Skyrim console command
	static void RunCommand(std::string command);

	// Try adding a custom command to a separate custom command queue.
	// Returns true if the addition was successful.
	// Returns false if the command is not a custom command and the caller
	// should add the command to another queue.
	static bool TryAddCustomCommand(const std::string &command);
	// a separate thread to run custom commands
	static DWORD WINAPI CustomCommandThread(void* ctx);
	// Register custom commands and start the thread that runs custom commands
	static void Initialize();

	// Add a new command: press <key name or DirectInput Scan Code> [millisecond]
	// Windows Virtual-Key Codes: https://www.creationkit.com/index.php?title=Input_Script#DXScanCodes
	//         Example: press m
	//                  press Z 1000
	//                  press 44 50
	//                  press 0x2C 100
	//                  press esc
	//         Press 3 keys at the same time (ctrl + alt + a):
	//                  press  ctrl 500  alt 500  a 400
	// Description: Simulate pressing the specified key the specified milliseconds.
	//              Used to cast skills or dragon shouts or do other actions.
	static void CustomCommandPress(std::vector<std::string> params);
};
