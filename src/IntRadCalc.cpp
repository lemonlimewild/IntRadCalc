/*
Virtual Operating System (V-OS) V1.0 for IntRadCalc
Can run custom-written programs off the SD
Manages display, SD card, LoRa, and button, display touch, and camera inputs
*/

#include <string>
#include <cstring>

//#include <Fonts\JetBrainsMono16.h>

#include "Initialization.h"
#include "Compiler.h"
#include "Interpreter.h"
#include "HomeScreenApp.h"
#define TFT_GREY 0x5AEB;

bool use16BitColorDepth = false;

void setup(void) {
  bool sdReady = sdSetup();
  bool tftReady = tftSetup(sdReady);
  Serial.begin(9600);
  Compiled testCompiled = compileToRAM(&HomeScreenApp);
  //logCompiledRAM(testCompiled);
  beginExecution(testCompiled);
  //loraSetup();
}

void loop() {}