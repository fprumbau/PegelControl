#ifndef CFG_H
#define CFG_H

//SPIFFS config disk
#include <ArduinoJson.h>
#include <FS.h>
#include "MyWifi.h"

class CFG {
  private:
  public:
        bool load();
        bool save();
};

#endif
