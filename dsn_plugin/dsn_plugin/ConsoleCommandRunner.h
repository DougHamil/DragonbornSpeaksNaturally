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
	// Windows DirectInput scan codes: https://www.creationkit.com/index.php?title=Input_Script#DXScanCodes
	// Avaliable key names: https://github.com/YihaoPeng/DragonbornSpeaksNaturally/blob/master/dsn_plugin/dsn_plugin/KeyCode.hpp
	//
	// Description: Simulate pressing the specified key the specified milliseconds.
	//              Used to cast skills or dragon shouts or do other actions.
	//
	//      Tips 1: If you want to use scan code 0-9, please add the prefix 0x, such as 0x01 for esc.
	//              A single digit without 0x prefix will be considered a digit key instead of a key code.
	//
	//      Tips 2: Only the currently active window can receive the simulated key.
	//              Activate the Skyrim window by calling activewindow command if necessary.
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

	// Add a new command: activewindow [window title|executable name]
	// 
	// Description: Activate the specified window.
	//              Used to switch to the correct window before running the press command.
	//              Omitting the parameters will activate the current Skyrim window.
	// 
	// Example:
	//         activewindow
	//         activewindow Notepad++
	//         activewindow notepad.exe
	//     Activate the Skyrim window and type in the console:
	//         activewindow; sleep 50; press a 10 e 10 i 10 o 10 u 10
	static void CustomCommandActiveWindow(std::vector<std::string> params);
};
