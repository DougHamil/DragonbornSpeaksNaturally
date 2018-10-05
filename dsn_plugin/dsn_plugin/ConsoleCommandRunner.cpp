#include "ConsoleCommandRunner.h"
#include "skse64/GameMenus.h"
#include "skse64/GameTypes.h"
#include "DSNMenuManager.h"
#include "KeyCode.hpp"
#include "WindowUtils.hpp"
#include "Log.h"

#include <sstream>
#include <algorithm>
#include <map>

static IMenu* consoleMenu = NULL;

std::vector<std::string> splitParams(std::string s) {
	for (size_t i = 0; i<s.size(); i++) {
		// replace blank characters to space
		if (s[i] == '\t' || s[i] == '\n' || s[i] == '\r' ||
			s[i] == '\0' || s[i] == '\x0B') {
			s[i] = ' ';
		}
	}

	std::stringstream ss(s);
	std::string item;
	std::vector<std::string> tokens;
	while (std::getline(ss, item, ' ')) {
		if (item.empty()) {
			continue;
		}
		tokens.push_back(item);
	}
	return tokens;
}

std::unordered_map<std::string/* name */, std::function<void(std::vector<std::string>)>/* func */> ConsoleCommandRunner::customCmdList;

void ConsoleCommandRunner::RunCommand(std::string command) {
	if (!consoleMenu) {
		Log::info("Trying to create Console menu");
		consoleMenu = DSNMenuManager::GetOrCreateMenu("Console");
	}

	if (consoleMenu != NULL) {
		//Log::info("Invoking command:");
		//Log::info(command);

		GFxValue methodName;
		methodName.type = GFxValue::kType_String;
		methodName.data.string = "ExecuteCommand";
		GFxValue resphash;
		resphash.type = GFxValue::kType_Number;
		resphash.data.number = -1;
		GFxValue commandVal;
		commandVal.type = GFxValue::kType_String;
		commandVal.data.string = command.c_str();
		GFxValue args[3];
		args[0] = methodName;
		args[1] = resphash;
		args[2] = commandVal;

		GFxValue resp;
		consoleMenu->view->Invoke("flash.external.ExternalInterface.call", &resp, args, 3);
	}
	else
	{
		Log::info("Unable to find Console menu");
	}
}

bool ConsoleCommandRunner::TryRunCustomCommand(const std::string & command) {
	std::vector<std::string> params = splitParams(command);
	
	if (params.empty()) {
		return false;
	}
	
	std::string action = params[0];
	// to lower
	for (size_t i = 0; i < action.size(); i++) {
		if ('A' <= action[i] && action[i] <= 'Z') {
			action[i] += 'a' - 'A';
		}
	}
	//Log::info(std::string("action: ") + action);

	auto itr = customCmdList.find(action);
	if (itr != customCmdList.end()) {
		itr->second(params);
		return true;
	}

	return false;
}

void ConsoleCommandRunner::RegisterCustomCommands() {
	// Register custom commands
	customCmdList["press"] = CustomCommandPress;
	customCmdList["sleep"] = CustomCommandSleep;
	customCmdList["activewindow"] = CustomCommandActiveWindow;
}

void ConsoleCommandRunner::CustomCommandPress(std::vector<std::string> params) {
	std::vector<UInt32 /*key*/> keyDown;
	std::map<UInt32 /*time*/, UInt32 /*key*/> keyUp;

	// If time does not exist, set as 50 milliseconds
	if ((params.size() - 1) % 2 > 0) {
		params.push_back("50");
	}

	// command: press <key> <time> <key> <time> ...
	//           [0]   [1]   [2]    [3]   [4]
	for (size_t i = 1; i + 1 < params.size(); i += 2) {
		const std::string &keyStr = params[i];
		const std::string &timeStr = params[i + 1];
		UInt32 key = 0;
		UInt32 time = 0;

		if (keyStr.empty()) {
			continue;
		}

		key = GetKeyScanCode(keyStr);
		if (key == 0) {
			continue;
		}

		time = strtol(timeStr.c_str(), NULL, 10);
		if (time == 0) {
			continue;
		}

		keyDown.push_back(key);

		// Map is used to sort by time.
		// Avoiding map key conflicts.
		// Although it changes the time, it is more convenient than sorting by myself.
		while (keyUp.find(time) != keyUp.end()) {
			time++;
		}
		keyUp[time] = key;
	}

	// send KEY_DOWN
	for (auto itr = keyDown.begin(); itr != keyDown.end(); itr++) {
		INPUT input;
		ZeroMemory(&input, sizeof(input));
		input.type = INPUT_KEYBOARD;
		input.ki.dwFlags = KEYEVENTF_SCANCODE;
		input.ki.wScan = *itr;

		SendInput(1, &input, sizeof(INPUT));
		//Log::info("key down: " + std::to_string(*itr));
	}

	// send KEY_UP
	int totalSleepTime = 0;
	for (auto itr = keyUp.begin(); itr != keyUp.end(); itr++) {
		int sleepTime = (*itr).first - totalSleepTime;
		Sleep(sleepTime);
		totalSleepTime += sleepTime;

		INPUT input;
		ZeroMemory(&input, sizeof(input));
		input.type = INPUT_KEYBOARD;
		input.ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
		input.ki.wScan = itr->second;

		SendInput(1, &input, sizeof(INPUT));
		//Log::info("key up: " + std::to_string(itr->second));
	}
}

void ConsoleCommandRunner::CustomCommandSleep(std::vector<std::string> params) {
	if (params.size() < 2) {
		return;
	}

	std::string &time = params[1];
	time_t millisecond = 0;

	if (time.size() > 2 && time[0] == '0' && (time[1] == 'x' || time[1] == 'X')) {
		// hex
		millisecond = strtol(time.substr(2).c_str(), NULL, 16);
	}
	else if ('0' <= time[0] && time[0] <= '9') {
		// dec
		millisecond = strtol(time.c_str(), NULL, 10);
	}

	if (millisecond > 0) {
		Sleep(millisecond);
	}
}

void ConsoleCommandRunner::CustomCommandActiveWindow(std::vector<std::string> params) {
	HWND window = NULL;
	DWORD pid = 0;
	std::string windowTitle;

	if (params.size() < 2) {
		pid = GetCurrentProcessId();
	}
	else {
		windowTitle = params[1];
		for (size_t i = 2; i < params.size(); i++) {
			windowTitle += ' ';
			windowTitle += params[i];
		}
		//Log::info("title: " + windowTitle);

		pid = GetProcessIDByName(windowTitle.c_str());
	}

	if (pid != 0) {
		window = FindMainWindow(pid);
		//Log::info("pid: "+std::to_string(pid) + ", window: " + std::to_string((uint64_t)window));
	}

	if (window == NULL && !windowTitle.empty()) {
		window = FindWindow(NULL, windowTitle.c_str());
		//Log::info("window: " + std::to_string((uint64_t)window));

		if (window == NULL) {
			window = FindWindow(windowTitle.c_str(), NULL);
			//Log::info("window: " + std::to_string((uint64_t)window));
		}
	}

	if (window != NULL) {
		SwitchToThisWindow(window, true);
	}
	else {
		Log::info("Cannot find windows with title/executable: " + windowTitle);
	}
}
