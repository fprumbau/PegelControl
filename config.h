#ifndef CFG_H
#define CFG_H

//SPIFFS config disk
#include <ArduinoJson.h>
#include <FS.h>
#include "MyWifi.h"

class CFG {
  private:
  public:
        const char* _ssid;
        const char* _password;
        bool load();
        bool save();
        void init(MyWifi& myWifi);
};

#endif
