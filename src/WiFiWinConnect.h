#pragma once
#include <WiFi.h>
#include "SettingsManager.h"

extern const char* ssid;
extern const char* password;

bool InitWiFi();