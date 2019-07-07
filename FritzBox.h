#ifndef FRITZ_H
#define FRITZ_H

#include <ArduinoFritzApi.h>

class Fritz {
  private:   
        bool _actorState = true; //Actor ist per Default ON, d.h. die Pumpe hat Strom
        bool _errorState = false; //In der Fritz-API ist ein Fehler aufgetreten, die letzte Aktion war erfolglos
  public:
        void init();
        bool startActor();
        bool stopActor();
        bool getActorState(); //true == on, false == off
        void print();
        void checkSetActor();
};


#endif
