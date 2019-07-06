#ifndef OTA_H
#define OTA_H

#include "global.h"
#include <ESPmDNS.h>

class OTA
{
  private:
    String _username = "";
    String _password = "";
    bool _authenticated;

    String _title = "OTA updater";
    String _banner = "OTA updater";
    String _build = "Build : ";
    String _branch = "Branch : ";
    String _deviceInfo = "ChipId : " + String(ESP.getChipRevision());
    String _footer = "ESP32 OTA Updater";
    void setup(const char *path, String username, String password);
    
  public:
    volatile bool stopForOTA = false;
    void init(const char* host);

    //setup web UI
    void setUpdaterUi(String title,String banner,String build,String branch,String deviceInfo,String footer);
    //setup web UI , with chip ID auto generated
    void setUpdaterUi(String title,String banner,String build,String branch,String footer);
    bool restartRequired = false; // Set this flag in the callbacks to restart ESP in the main loop
};

/*
#include <ESPmDNS.h>
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
*/

#endif
