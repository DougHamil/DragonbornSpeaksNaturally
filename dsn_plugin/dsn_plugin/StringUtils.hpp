#pragma once
#include <string>
#include <sstream>
#include <vector>

static std::vector<std::string> splitParams(std::string s) {
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

inline static void stringToLower(std::string &str) {
	for (size_t i = 0; i < str.size(); i++) {
		if ('A' <= str[i] && str[i] <= 'Z') {
			str[i] += 'a' - 'A';
		}
	}
}
