#pragma once
#include "common/IPrefix.h"
#include "skse64/GameMenus.h"
#include <string>
#include <vector>
#include <mutex>
#include <queue>
#include <Windows.h>

class ConsoleCommandRunner
{
private:
	static const int kMaxCustomCmdQueueSize = 1000;

	static std::mutex customCmdQueueLock;
	static HANDLE customCmdQueueSemaphore;
	static std::queue<std::vector<std::string>> customCmdQueue;

public:
	static void RunCommand(std::string command);
	static bool TryRunCustomCommand(const std::string &command);
	static DWORD WINAPI RunCustomCommandThread(void* ctx);
	static void Initialize();

	// Add a new command: press <character or VirtualKeyCode> <millisecond>
	// Windows Virtual-Key Codes: https://msdn.microsoft.com/en-us/library/dd375731(VS.85).aspx
	//           Example: press    Z 1000
	//                    press  127 1000
	//                    press 0x58 1000
	//               Press 4 keys at the same time:
	//                    press   a 1000   127 500   z 1000   0x58 3000
	// Description: Simulate pressing the specified key the specified milliseconds.
	//              Used to cast skills or dragon shouts or do other actions.
	static void RunCustomCommandPress(const std::vector<std::string> &params);
};

