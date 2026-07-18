#pragma once
#include "AppHeader.h"

extern const AppHeader HomeScreenApp;

static const char* const appContent[] = {
  "win.maximize $main",
  "func Main []",
  "new w",
  "new h",
  "win.width w $main",
  "win.height h $main",
  "disp.rect 0x0F00F0 0 0 w h",
  "new textX",
  "new textY",
  "set textX 0",
  "set textY 0",
  "add textX textX 10",
  "add textY textY 30",
  "disp.text 0x00FF00 \"Some Text\" 20 30 2",
  "funcEnd",
  "event.win.changedSize $main",
  "event.end",
  "call Main []"
};

const AppHeader HomeScreenApp = {
  0,
  "Home Screen",
  1.0,
  0x0000000000000000,
  appContent,
  sizeof(appContent) / sizeof(*appContent)
};