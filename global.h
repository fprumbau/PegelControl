#ifndef GLOBAL_H
#define GLOBAL_H

#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <SPIFFS.h>
#include <Update.h>
#include <time.h>
#include <FS.h> 

#include "FritzBox.h"
#include "config.h"
#include "MyWifi.h"
#include "Log.h"
#include "OTA.h"


extern CFG config;
extern MyWifi myWifi;
extern AsyncWebServer server;   
extern AsyncWebSocket ws;
extern OTA updater; //Over-the-air updater
extern WiFiUDP udp;
extern LOG logs; //Ringspeicher, Groesse in Log.h definiert
extern const char* hostName;
extern Fritz fritz;

#endif
