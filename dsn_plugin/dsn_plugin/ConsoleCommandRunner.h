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
	static std::unordered_map<std::string/* name */, std::function<void(std::vector<std::string>)>/* func */> customCmdList;

public:
	// Run a Skyrim console command
	static void RunCommand(std::string command);

	// Register custom commands
	static void RegisterCustomCommands();
	// Try run a custom command.
	// Returns true if the command was running successful.
	// Returns false if the command is not a custom command and the caller
	// should add the command to another queue.
	static bool TryRunCustomCommand(const std::string &command);

	// Add a new command: press <key name or DirectInput Scan Code> [millisecond]
	// Windows Virtual-Key Codes: https://www.creationkit.com/index.php?title=Input_Script#DXScanCodes
	//
	// Description: Simulate pressing the specified key the specified milliseconds.
	//              Used to cast skills or dragon shouts or do other actions.
	//
	// Example:
	//         press m
	//         press Z 1000
	//         press 44 50
	//         press 0x2C 100
	//         press esc
	//    Press 3 keys at the same time (ctrl + alt + a):
	//         press  ctrl 500  alt 500  a 400
	static void CustomCommandPress(std::vector<std::string> params);

	// Add a new command: sleep millisecond
	// 
	// Description: Delays the subsequent commands the specified milliseconds.
	//              Used to form an automatic action script with the press command.
	//              Both custom commands and Skyrim commands can be delayed.
	// 
	// Example:
	//         sleep 5000
	//         press z; sleep 1000; press LeftMouseButton 2000
	//         press ctrl; sleep 5000; press ctrl
	//     Casting two dragon shouts one after another:
	//         player.cast 0003f9ed player voice; sleep 3000; player.cast 00013f3a player voice
	static void CustomCommandSleep(std::vector<std::string> params);
};
