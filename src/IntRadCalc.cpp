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

bool sdReady;
bool tftReady;
bool obfsReady;
float softwareVersion = 1.0;
bool use16BitColorDepth = false;

void setup(void) {
  sdReady = sdSetup();
  tftReady = tftSetup(sdReady);
  obfsReady = initOBFS();

  Serial.begin(9600);
  Compiled testCompiled = compileToRAM(&HomeScreenApp);
  logCompiledRAM(testCompiled);
  //beginExecution(testCompiled);
  //loraSetup();
}

void loop() {}