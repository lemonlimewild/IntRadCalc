#pragma once

#ifndef INITIALIZATION_H
#define INITIALIZATION_H

#include <Arduino.h>
#include <FS.h>
#include <SD.h>
#include <SPI.h>
#include <TFT_eSPI.h>

#include "Console.h"

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

#endif

void loraSetup();
bool sdSetup();
void tftSetup(bool sdReady);