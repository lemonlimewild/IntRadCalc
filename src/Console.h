#pragma once

#ifndef CONSOLE_H
#define CONSOLE_H

#include <TFT_eSPI.h>
#include <stdlib.h>

extern bool consoleOpen;
extern const uint8_t consoleLines;
extern TFT_eSprite frameBuffer;
extern bool useFrameBuffer;
extern TFT_eSPI tft;

void logToConsole(char* message);
void logToConsole(const char* message);

#endif