#ifndef MYWIFI_H
#define MYWIFI_H

#include <ESP8266WiFi.h>

class MyWifi {
    private:
      String _ip;
    public:
      const char* _ssid = "";
      const char* _password = "";
      void connect();
      String getIpAddress();
};

#endif
