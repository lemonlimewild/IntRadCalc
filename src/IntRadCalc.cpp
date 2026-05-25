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

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite frameBuffer = TFT_eSprite(&tft); //create a frame buffer sprite
HardwareSerial& loraSerial = Serial1;
std::string softwareVersion = "1.0";

//pin definitions
//const uint16_t TFT_MOSI = 12;
//const uint16_t TFT_MISO = 10;
//const uint16_t TFT_SCLK = 18;
//const uint16_t TFT_CS = 14;
//const uint16_t TFT_DC = 13;
//const uint16_t TFT_RST = 9;
//const uint16_t TFT_BL = 11;

//const uint16_t TOUCH_CS = 7;
//const uint16_t TOUCH_IRQ = 6;

const uint16_t SD_CS = 17;

const uint16_t LORA_DOUT = 48; //data from LoRa (MCU RX)
const uint16_t LORA_DIN = 47; //data to loRa (MCU TX)
const uint16_t LORA_RST = 38;

const uint16_t BTN_PL = 4; //parallel load
const uint16_t BTN_CLK = 5; //clock to view next (B16 -> B1)
const uint16_t BTN_SOUT = 8; //serial out

//time vars
uint32_t fsMillis = 0; //frame start milliseconds
uint32_t fsMicros = 0; //frame start microseconds

//button control vars
uint16_t btnScanFreq = 10000; //scan buttons every X microseconds, 10k uS = 100Hz
uint64_t lastBtnScanTime = 0; //last button scan time in microseconds
bool buttonStates[16] = {}; //holds current button states

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

void buttonSetup() {
  pinMode(BTN_PL, OUTPUT);
  pinMode(BTN_CLK, OUTPUT);
  pinMode(BTN_SOUT, INPUT);

  digitalWrite(BTN_PL, HIGH);
  digitalWrite(BTN_CLK, LOW); //low to high transition clocks
}

void scanButtons() {
  //load parallel button inputs into parallel to serial converters
  digitalWrite(BTN_PL, LOW);
  delayMicroseconds(1);
  digitalWrite(BTN_PL, HIGH);

  //clock and store all button states, from button 16 down to 1
  for (uint8_t i = 0; i < 16; i++) {
    digitalWrite(BTN_CLK, HIGH);
    delayMicroseconds(1);
    buttonStates[15 - i] = digitalRead(BTN_SOUT); //read after rising edge
    digitalWrite(BTN_CLK, LOW);
    delayMicroseconds(1);
  }
}

void setup(void) {
  bool SDAvailable = tftAndSDSetup();
  buttonSetup();
  Serial.begin(9600);
  //loraSetup();
  //setupApplication();
}

void loop() {
  fsMillis = millis();
  fsMicros = micros();

  //button logic
  if (fsMicros - lastBtnScanTime > btnScanFreq) {
    scanButtons();
    lastBtnScanTime = fsMicros;
  }

  //executeApplication();
}