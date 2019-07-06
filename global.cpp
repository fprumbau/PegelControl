#include "global.h"

#include "MyWifi.h"
#include "OTA.h"


CFG config;
MyWifi myWifi;
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
OTA updater;
WiFiUDP udp;
LOG logs;
const char* hostName = "esp32a";
Fritz fritz;
