#include "Initialization.h"

TFT_eSPI tft = TFT_eSPI();
bool useFrameBuffer = true;
TFT_eSprite frameBuffer = TFT_eSprite(&tft); //create a frame buffer sprite

HardwareSerial& loraSerial = Serial1;

std::string softwareVersion = "1.0";

void loraSetup() {
  //loraSerial.begin(9600, SERIAL_8N1, LORA_DOUT, LORA_DIN); //to do
}

bool sdSetup() {
  SPI.begin(TFT_SCLK, TFT_MISO, TFT_MOSI, SD_CS);
  bool sdInit = SD.begin(SD_CS);
  return sdInit;
}

void tftSetup(bool sdReady) {
  pinMode(TFT_BL, OUTPUT);
  pinMode(TFT_RST, OUTPUT);
  analogWrite(TFT_BL, 128);
  tft.init();
  tft.setRotation(3);
  frameBuffer.setColorDepth(16);
  frameBuffer.createSprite(tft.width(), tft.height());
  logToConsole(("Running version: " + softwareVersion).data());
  logToConsole(("Display Resolution: " + std::to_string(tft.width()) + " x " + std::to_string(tft.height()) + " pixels").data());
  if (sdReady) {
    logToConsole("SD status: Connected");
  } else {
    logToConsole("SD status: Failed");
  }
}
