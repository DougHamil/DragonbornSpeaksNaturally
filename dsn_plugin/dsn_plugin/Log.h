#pragma once
#include "common/IPrefix.h"
#include <cstdlib>
#include <string>

class Log
{
public:
	static Log* get();
	Log();
	~Log();
	static Log* instance;

	static void info(std::string message);
	static void address(std::string message, uintptr_t addr);
	static void hex(std::string message, uintptr_t addr);
};