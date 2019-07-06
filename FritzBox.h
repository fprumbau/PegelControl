#ifndef FRITZ_H
#define FRITZ_H

#include <ArduinoFritzApi.h>

class Fritz {
  private:   
        //const char* fritz_user = "admin";
        //const char* fritz_password = "shitonstupp";
  public:
        void init();
        bool startActor();
        bool stopActor();
        void print();
        void checkValues();
};


#endif
