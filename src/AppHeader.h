#pragma once
#include <cstdint>

struct AppHeader {
	const uint8_t appType; //0-256
	const char* appName;
	const double minOSVer; //runs on any higher version
	const uint16_t permissions; //all permissions app wants, the ones that are granted are stored separately
	const char* const* content;
	const uint32_t contentLines;
};