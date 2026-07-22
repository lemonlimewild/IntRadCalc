//Connect to Windows for control
//Uses a TCP server

#include "WiFiWinConnect.h"

uint32_t attemptConnectionTime = 20000;

WiFiServer server(4242); //TODOLATER possibly change port
WiFiClient client;

bool InitWiFi() {
    WiFi.begin(ssid, password);
    unsigned long startTime = millis();
    while(WiFi.status() != WL_CONNECTED || millis() - startTime > attemptConnectionTime) {
        //wait on a connection or timeout
    }
    if (WiFi.status() != WL_CONNECTED) return false;
}