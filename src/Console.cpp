#include "Console.h"

bool consoleOpen = true;
const uint8_t consoleLineCount = 15;
char* consoleLine[consoleLineCount] = {};

//make sure to run after display setup only!
void renderConsole() {
    if (!consoleOpen) return;
    if (useFrameBuffer) {
        frameBuffer.fillRect(0, tft.height() - consoleHeight, tft.width(), consoleHeight, TFT_BLACK);
        frameBuffer.setTextColor(TFT_GREEN);
        for (uint8_t i = 0; i < consoleLineCount; i++) {
            if (consoleLine[i]) {
                frameBuffer.drawString(consoleLine[i], 10, tft.height() - i * 20 - 30, 2);
            }
        }
        frameBuffer.pushSprite(0, 0);
    } else {
        //TODO no framebuffer console rendering
    }
}

void logToConsole(const char* message, bool noNewLine) { //char* is automatically converted to const char*, both types safe
    //only responsible for allocating and freeing elements in consoleLines
    if (!message) return;
    if (!noNewLine) {
        if (consoleLine[consoleLineCount - 1]) free(consoleLine[consoleLineCount - 1]);
        for (uint8_t i = consoleLineCount - 1; i > 0; i--) {
            consoleLine[i] = consoleLine[i - 1];
        }
        uint16_t messageLen = 0;
        while(message[messageLen] != '\0') messageLen++;
        char* messageCopy = (char*)malloc(sizeof(char) * (messageLen + 1));
        if (!messageCopy) return;
        for (uint16_t i = 0; i < messageLen; i++) {
            messageCopy[i] = message[i];
        }
        consoleLine[0] = messageCopy;
    } else {
        if (!consoleLine[0]) return;
        uint16_t lastLength = 0;
        uint16_t appendLength = 0;
        while (consoleLine[0][lastLength] != '\0') lastLength++;
        while (consoleLine[0][appendLength] != '\0') appendLength++;
        char* newMessage = (char*)malloc(lastLength + appendLength);
        uint16_t index = 0;
        while (index < lastLength) {
            newMessage[index] = consoleLine[0][index];
            index++;
        }
        index = 0;
        while (index < appendLength) {
            newMessage[lastLength + index] = message[index];
            index++;
        }
        free(consoleLine[0]);
        consoleLine[0] = newMessage;
    }
    renderConsole();
}