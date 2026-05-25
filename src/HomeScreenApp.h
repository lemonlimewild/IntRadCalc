#pragma once
#include "AppHeader.h"

extern const AppHeader HomeScreenApp;

static const char* const appContent[] = {
  "func OnProgStart input",
  "sys.disp.rect 0x000000 0 0 sys.disp.dispWidth sys.disp.dispHeight",
  "sys.disp.text 0x00FF00 input 10 10 2",
  "funcEnd",
  "func Main",
  "new execCountDoubled",
  "set execCountDoubled sys.execCount",
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