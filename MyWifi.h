#ifndef MYWIFI_H
#define MYWIFI_H

#include <WiFi.h>
#include <WiFiClient.h>

class MyWifi {
    private:
      String _ip;
      IPAddress _localIP;
    public:
      const char* _ssid;
      const char* _password;
      void connect();
      String getIpAddress();
      IPAddress localIP();
      void reconnect();
};

#endif
