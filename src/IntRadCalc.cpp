/*
Virtual Operating System (V-OS) V1.0 for IntRadCalc
Can run custom-written programs off the SD
Manages display, SD card, LoRa, and button, display touch, and camera inputs
*/

#include <Arduino.h>
#include <FS.h>
#include <SD.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <string>
#include <cstring>

//#include <Fonts\JetBrainsMono16.h>

#include "Compiler.h"
#include "HomeScreenApp.h"

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite frameBuffer = TFT_eSprite(&tft); //create a frame buffer sprite
HardwareSerial& loraSerial = Serial1;
std::string softwareVersion = "1.0";

//pin definitions for reference
//TFT_MOSI = 12;
//TFT_MISO = 10;
//TFT_SCLK = 18;
//TFT_CS = 14;
//TFT_DC = 13;
//TFT_RST = 9;
//TFT_BL = 11;
//TOUCH_CS = 7;
//TOUCH_IRQ = 6;

const uint16_t SD_CS = 17;

const uint16_t LORA_DOUT = 48; //data from LoRa (MCU RX)
const uint16_t LORA_DIN = 47; //data to loRa (MCU TX)
const uint16_t LORA_RST = 38;

const uint16_t BTN_PL = 4; //parallel load
const uint16_t BTN_CLK = 5; //clock to view next (B16 -> B1)
const uint16_t BTN_SOUT = 8; //serial out

#define TFT_GREY 0x5AEB;

void loraSetup() {
  //loraSerial.begin(9600, SERIAL_8N1, LORA_DOUT, LORA_DIN); //to do
}

int tftAndSDSetup() {
  //attempt initializing SD card SPI, shared with the dislay and touch interface
  SPI.begin(TFT_SCLK, TFT_MISO, TFT_MOSI, SD_CS);
  bool sdInit = SD.begin(SD_CS);
  
  //initialize the display
  pinMode(TFT_BL, OUTPUT);
  pinMode(TFT_RST, OUTPUT);
  analogWrite(TFT_BL, 128);
  tft.init();
  tft.setRotation(3);
  frameBuffer.setColorDepth(16);
  frameBuffer.createSprite(tft.width(), tft.height());

  //log to display
  frameBuffer.fillScreen(TFT_BLACK);
  frameBuffer.setTextColor(TFT_GREEN);
  frameBuffer.drawString(("Running version: " + softwareVersion).data(), 10, 10, 2);//&JetBrainsMono16);
  frameBuffer.drawString(("Display Resolution: " + std::to_string(tft.width()) + " x " + std::to_string(tft.height()) + " pixels").data(), 10, 30, 2);
  frameBuffer.drawString("SD SPI Configured", 10, 50, 2);
  if (sdInit) {
    frameBuffer.drawString("SD status: Connected", 10, 70, 2);
  } else {
    frameBuffer.drawString("SD status: Failed", 10, 70, 2);
  }

  frameBuffer.pushSprite(0, 0);

  return sdInit;
}

void setup(void) {
  bool SDAvailable = tftAndSDSetup();
  Serial.begin(9600);
  Compiled testCompiled = compileToRAM(&HomeScreenApp);
  //loraSetup();
}

void loop() {}