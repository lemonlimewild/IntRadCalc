#include "Console.h"
//#include <TFT_eSPI.h>
//#include <stdlib.h>

bool consoleOpen = true;
const uint8_t consoleLineCount = 15;
char* consoleLine[consoleLineCount];
uint16_t consoleHeight = tft.height();

extern std::string softwareVersion;

void renderConsole() {
    if (useFrameBuffer) {
        frameBuffer.fillRect(0, tft.height() - consoleHeight, tft.width(), consoleHeight, TFT_BLACK);
        frameBuffer.setTextColor(TFT_GREEN);
        for (uint8_t i = 0; i < consoleLineCount; i++) {
            frameBuffer.drawString(consoleLine[consoleLineCount - i - 1], 10, 20 * i - 10, 2);
        }
        frameBuffer.pushSprite(0, 0);
    } else {
        //TODO no framebuffer console rendering
    }
}

void logToConsole(char* message) {
    if (consoleLine[consoleLineCount - 1]) free(consoleLine[consoleLineCount - 1]);
    for (uint8_t i = consoleLineCount - 1; i > 0; i--) {
        consoleLine[i] = consoleLine[i - 1];
    }
    consoleLine[0] = message;
    if (consoleOpen) {
        renderConsole();
    }
}

void logToConsole(const char* message) {
    if (consoleLine[consoleLineCount - 1]) free(consoleLine[consoleLineCount - 1]);
    for (uint8_t i = consoleLineCount - 1; i > 0; i--) {
        consoleLine[i] = consoleLine[i - 1];
    }
    uint16_t messageLen = 0;
    while(message[messageLen] != '\0') messageLen++;
    char* mutuableMessage = (char*)malloc(sizeof(char) * (messageLen + 1));
    for (uint16_t i = 0; i < messageLen; i++) {
        mutuableMessage[i] = message[i];
    }
    consoleLine[0] = mutuableMessage;
    if (consoleOpen) {
        renderConsole();
    }
}