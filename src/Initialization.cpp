#include "Initialization.h"

//contains initialization functions

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite frameBuffer = TFT_eSprite(&tft); // create a frame buffer sprite

HardwareSerial &loraSerial = Serial1;

extern float softwareVersion;

void loraSetup() {
	// loraSerial.begin(9600, SERIAL_8N1, LORA_DOUT, LORA_DIN); //to do
}

bool sdSetup() {
	SPI.begin(TFT_SCLK, TFT_MISO, TFT_MOSI, SD_CS);
	bool sdInit = SD.begin(SD_CS);
	return sdInit;
}

void resizeWindows() {
	
}

bool tftSetup(bool sdReady) {
	pinMode(TFT_BL, OUTPUT);
	pinMode(TFT_RST, OUTPUT);
	analogWrite(TFT_BL, 128);
	tft.init();
	tft.setRotation(3);
	if (use16BitColorDepth) {
		frameBuffer.setColorDepth(16);
	}
	else {
		frameBuffer.setColorDepth(8);
	}
	frameBuffer.createSprite(tft.width(), tft.height());
	if (!frameBuffer.created()) {
		return false;
	}

	tft.fillScreen(TFT_SKYBLUE);
	changeConsoleSize(tft.height() / 2); //init the console
	changeConsoleLogDelay(500);
	//TODOLATER apply new logging format, unuglify
	logToConsole((std::string("Running version: ") + std::to_string(softwareVersion)).c_str());
	logToConsole((std::string("Display Resolution: ") + std::to_string(tft.width()) + std::string(" x ") + std::to_string(tft.height()) + std::string(" pixels")).c_str());
	if (sdReady) {
		logToConsole("SD status: Connected");
	}
	else {
		logToConsole("SD status: Failed");
	}

	return true;
}

bool initOBFS() {
    bool isOBFSReady = false;

    logToConsole("Initiating Onboard Filesystem...");
    uint8_t attemptCounter = 0;
    while (attemptCounter < 5 && !isOBFSReady) {
        logToConsole("Attempt ");
        logToConsole(attemptCounter + 1, true);
        isOBFSReady = LittleFS.begin();
        if (!isOBFSReady) {
            delay(500);
        }
    }
    
    if (isOBFSReady) {
        logToConsole("Filesystem Initiated.");
    } else {
        logToConsole("Failed to Initiate Filesystem.");
        logToConsole("Attempting to Format...");
        
        isOBFSReady = LittleFS.begin(true);
        if (isOBFSReady) {
            logToConsole("Formatting Succeeded.");
        } else {
            logToConsole("Formatting Failed. Filesystem not Connected.");
        }
    }

    return isOBFSReady;
}