#pragma once

#ifndef CONSOLE_H
#define CONSOLE_H

#include <TFT_eSPI.h>
#include <stdlib.h>

extern bool consoleOpen;
extern uint16_t consoleHeight;
extern const uint8_t consoleLineCount;
extern TFT_eSprite frameBuffer;
extern bool useFrameBuffer;
extern TFT_eSPI tft;
extern std::string softwareVersion;

void logToConsole(const char* message, bool noNewLine = false);
void renderConsole();

#endif //CONSOLE_H