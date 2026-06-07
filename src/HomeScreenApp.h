#pragma once
#include "AppHeader.h"

extern const AppHeader HomeScreenApp;

static const char* const appContent[] = {
  "new dimensions",
  "set dimensions sys.disp.windowSize",
  "func Main",
  "disp.makeWindow",
  "disp.setWindowMaximized true",
  "disp.rect 0x000000 dimensions[0] dimensions[1] dimensions[2] dimensions[3]",
  "new textX",
  "new textY",
  "set textX dimensions[0]",
  "set textY dimensions[1]",
  "add textX 10",
  "add textY 30",
  "disp.text 0x00FF00 \"Some Text\" dimensions[0] dimensions[1] 2",
  "funcEnd",
  "eventHandle sys.events.windowResize",
  "eventHandleEnd"
};
/*static const char* const appContent[] = {
  "func OnProgStart input",
  "sys.disp.rect 0x000000 0 0 sys.disp.dispWidth sys.disp.dispHeight",
  "sys.disp.text 0x00FF00 input 10 10 2",
  "new execCountDoubled",
  "set execCountDoubled 5",
  "funcEnd",
  "func Main",
  "new execCountDoubled",
  "set execCountDoubled sys.app.execCount",
  "mul execCountDoubled execCountDoubled 2",
  "intToStr execCountDoubled",
  "sys.disp.rect 0x000000 20 30 100 50",
  "sys.disp.text 0x00FF00 execCountDoubled 20 30 2",
  "funcEnd",
  "call OnProgStart \"Test Code\"",
  "call Main"
};
/*static const char* const appContent[] = {
	"func Main",
	"newInt a",
	"set a 5",
	"funcEnd"
};*/

const AppHeader HomeScreenApp = {
  0,
  "Home Screen",
  1.0,
  0x0000000000000000,
  appContent,
  sizeof(appContent) / sizeof(*appContent)
};