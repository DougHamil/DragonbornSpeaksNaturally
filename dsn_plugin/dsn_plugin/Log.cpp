#include "Log.h"
#include <sstream>
#include <fstream>

static bool LOG_ENABLED = true;

Log::Log()
{
}

Log::~Log()
{
}

Log* Log::instance = NULL;

Log* Log::get() {
	if (!instance)
		instance = new Log();
	return instance;
}

void Log::info(std::string message) {
	std::ofstream log_file(
		"dragonborn_speaks.log", std::ios_base::out | std::ios_base::app);
	log_file << message << std::endl;
}

void Log::address(std::string message, uintptr_t addr) {
	std::stringstream ss;
	ss << std::hex << addr;
	const std::string s = ss.str();
	Log::info(message.append(s));
}

void Log::hex(std::string message, uintptr_t addr) {
	std::stringstream ss;
	ss << std::hex << addr;
	const std::string s = ss.str();
	Log::info(message.append(s));
}