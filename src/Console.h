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
extern bool consoleOpen;
extern const uint8_t consoleLineCount;
extern float softwareVersion;

void resizeWindows();
void logToConsole(const char* message, bool noNewLine = false);
void logToConsole(double message, bool noNewLine = false);
void renderConsole();
void toggleConsole();
void openConsole();
void closeConsole();
bool changeConsoleSize(uint16_t newSize);
void changeConsoleLogDelay(uint32_t newDelay);

#endif //CONSOLE_H