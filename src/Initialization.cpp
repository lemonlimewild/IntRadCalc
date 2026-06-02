#include "Initialization.h"

TFT_eSPI tft = TFT_eSPI();
bool useFrameBuffer = true;
TFT_eSprite frameBuffer = TFT_eSprite(&tft); //create a frame buffer sprite

HardwareSerial& loraSerial = Serial1;

std::string softwareVersion = "1.0";

uint16_t consoleHeight;

void loraSetup() {
  //loraSerial.begin(9600, SERIAL_8N1, LORA_DOUT, LORA_DIN); //to do
}

bool sdSetup() {
  SPI.begin(TFT_SCLK, TFT_MISO, TFT_MOSI, SD_CS);
  bool sdInit = SD.begin(SD_CS);
  return sdInit;
}

bool tftSetup(bool sdReady, bool use16BitColorDepth) {
  pinMode(TFT_BL, OUTPUT);
  pinMode(TFT_RST, OUTPUT);
  analogWrite(TFT_BL, 128);
  tft.init();
  tft.setRotation(3);
  if (use16BitColorDepth) {
    frameBuffer.setColorDepth(16);
  } else {
    frameBuffer.setColorDepth(8);
  }
  frameBuffer.createSprite(tft.width(), tft.height());
  if (!frameBuffer.created()) {
    return false;
  }

  consoleHeight = tft.height() / 2;

  logToConsole(("Running version: " + softwareVersion + "\0").data());
  logToConsole(("Display Resolution: " + std::to_string(tft.width()) + " x " + std::to_string(tft.height()) + " pixels\0").data());
  if (sdReady) {
    logToConsole("SD status: Connected\0");
  } else {
    logToConsole("SD status: Failed\0");
  }
  renderConsole();
  return true;
}
