#ifndef OTA_H
#define OTA_H

#include <Esp.h>
#include <WString.h>
#include <ESP8266OTA.h>
#include <ESP8266mDNS.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

class OTA {
   private:
      ESP8266OTA _otaUpdater;
   public:
      void init(ESP8266WebServer& server, const char* host, const char* update_path, const char* update_username, const char* update_password, int FW_VERSION);
};

#endif
