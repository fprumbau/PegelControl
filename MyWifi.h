#ifndef MYWIFI_H
#define MYWIFI_H

#include <ESP8266WiFi.h>

class MyWifi {
    private:
      String _ip;
    public:
      char* _ssid = "";
      char* _password = "";
      void connect();
      String getIpAddress();
};

#endif
