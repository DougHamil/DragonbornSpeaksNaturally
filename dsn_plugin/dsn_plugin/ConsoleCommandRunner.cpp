#include "ConsoleCommandRunner.h"
#include "skse64/GameMenus.h"
#include "DSNMenuManager.h"
#include "skse64/GameTypes.h"
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

std::mutex ConsoleCommandRunner::customCmdQueueLock;
HANDLE ConsoleCommandRunner::customCmdQueueSemaphore;
std::queue<std::vector<std::string>> ConsoleCommandRunner::customCmdQueue;

void ConsoleCommandRunner::RunCommand(std::string command) {

	if (TryRunCustomCommand(command)) {
		return;
	}

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
	Log::info(std::string("action: ") + action);

	if (action == "press") {
		// mutex is automatically released when scopeLock goes out of scope
		std::lock_guard<std::mutex> scopeLock(customCmdQueueLock);

		// customCmdQueueSemaphore++
		if (ReleaseSemaphore(customCmdQueueSemaphore, 1, NULL)) {
			customCmdQueue.push(params);
		}

		return true;
	}

	return false;
}

DWORD WINAPI ConsoleCommandRunner::RunCustomCommandThread(void* ctx) {
	customCmdQueueSemaphore = CreateSemaphore(NULL, 0, kMaxCustomCmdQueueSize, NULL);

	for (;;) {
		// waiting for customCmdQueueSemaphore > 0, then customCmdQueueSemaphore--
		WaitForSingleObject(customCmdQueueSemaphore, INFINITE);

		// mutex is automatically released when scopeLock goes out of scope
		std::lock_guard<std::mutex> scopeLock(customCmdQueueLock);

		std::vector<std::string> params = customCmdQueue.front();
		customCmdQueue.pop();

		if (params[0] == "press") {
			RunCustomCommandPress(params);
		}
	}
}

void ConsoleCommandRunner::Initialize() {
	CreateThread(NULL, 0, ConsoleCommandRunner::RunCustomCommandThread, NULL, 0L, NULL);
}

void ConsoleCommandRunner::RunCustomCommandPress(const std::vector<std::string>& params) {
	std::vector<int /*key*/> keyDown;
	std::map<int /*time*/, int /*key*/> keyUp;

	// command: press <key> <time> <key> <time> ...
	//           [0]   [1]   [2]    [3]   [4]
	for (size_t i = 1; i + 1 < params.size(); i += 2) {
		const std::string &keyStr = params[i];
		const std::string &timeStr = params[i + 1];
		int key = 0;
		int time = 0;

		if (keyStr.empty()) {
			continue;
		}

		if (keyStr.size() > 2 && keyStr[0] == '0' && (keyStr[1] == 'x' || keyStr[1] == 'X')) {
			// key code hex
			key = strtol(keyStr.substr(2).c_str(), NULL, 16);
		}
		else if ('0' <= keyStr[0] && keyStr[0] <= '9') {
			// key code dec
			key = strtol(keyStr.c_str(), NULL, 10);
		}
		else if ('A' <= keyStr[0] && keyStr[0] <= 'Z') {
			// character
			key = (int)keyStr[0];
		}
		else if ('a' <= keyStr[0] && keyStr[0] <= 'z') {
			// lower character, to upper
			key = (int)(keyStr[0] - ('a' - 'A'));
		}
		else {
			continue;
		}

		time = strtol(timeStr.c_str(), NULL, 10);

		if (time <= 0) {
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

		// send KEY_DOWN
		for (auto itr = keyDown.begin(); itr != keyDown.end(); itr++) {
			INPUT input;
			ZeroMemory(&input, sizeof(input));

			input.type = INPUT_KEYBOARD;
			input.ki.wVk = *itr;
			SendInput(1, &input, sizeof(INPUT));
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
			input.ki.wVk = (*itr).second;
			input.ki.dwFlags = KEYEVENTF_KEYUP;
			SendInput(1, &input, sizeof(INPUT));
		}
	}
}
