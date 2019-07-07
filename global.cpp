#include "global.h"

#include "MyWifi.h"
#include "OTA.h"

bool debug = false;
int level = 0;

CFG config;
MyWifi myWifi;
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
OTA updater;
WiFiUDP udp;
LOG logs;
const char* hostName = "esp32wrover";
Fritz fritz;
const char* fritz_user     = "admin";
const char* fritz_password = "shitonstupp";
const char* fritz_ip       = "192.168.178.1";
const char* fritz_ain      = "F0:B0:14:80:91:6D";
FritzApi api(fritz_user, fritz_password, fritz_ip);  
bool test = false;
String testData;

//Werte entscheiden ueber Steckdose des Wasserwerks
int pegelLimit;
int temperatureLimit;
int pegel = -1;
int temperature = -1;
int relayStatus = 0; //Default, d.h. alles OK
unsigned long lastActorAction;
unsigned long minRelaisActorInterval=300000; //Hoechstens alle 5 Minuten
