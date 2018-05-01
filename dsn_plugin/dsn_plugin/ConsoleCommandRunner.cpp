#include "ConsoleCommandRunner.h"
#include "skse64/GameMenus.h"
#include "DSNMenuManager.h"
#include "skse64/GameTypes.h"
#include "Log.h"

static IMenu* consoleMenu = NULL;

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