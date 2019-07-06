#ifndef MYWIFI_H
#define MYWIFI_H

#include <WiFi.h>
#include <WiFiClient.h>

class MyWifi {
    private:
      String _ip;
      IPAddress _localIP;
      const char* _ssid = "Perry";
      const char* _password = "5785340536886787";
    public:
      //const char* _ssid = "";
      //const char* _password = "";
      void connect();
      String getIpAddress();
      IPAddress localIP();
      void reconnect();
};

#endif
