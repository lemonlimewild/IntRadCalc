#pragma once

#ifndef CONSOLE_H
#define CONSOLE_H
#include <stdlib.h>
#include <TFT_eSPI.h>

//need definitions for .cpp file
/*//forward-declare instead of pasting full lib contents
class TFT_eSPI;
class TFT_eSprite;*/

extern TFT_eSPI tft;
extern TFT_eSprite frameBuffer;
extern bool consoleOpen;
extern uint16_t consoleHeight;
extern const uint8_t consoleLineCount;
extern bool useFrameBuffer;
extern float softwareVersion;

void logToConsole(const char* message, bool noNewLine = false);
void renderConsole();

#endif //CONSOLE_H