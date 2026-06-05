#include "Initialization.h"

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite frameBuffer = TFT_eSprite(&tft); // create a frame buffer sprite

HardwareSerial &loraSerial = Serial1;

float softwareVersion = 1.0;

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
	changeConsoleLogDelay(1000);
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
