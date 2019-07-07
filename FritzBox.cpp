#include "FritzBox.h"

#include <ArduinoFritzApi.h>

#include "global.h"

void Fritz::init() {
  //api = FritzApi(fritz_user, fritz_password, fritz_ip)&; 
  api.init();

  _actorState = getActorState();
  if(!_errorState) {
    Serial.print("Auslesen der Pumpensteckdose (An==1, Aus==0): ");
    Serial.println(_actorState);
  } else {
    Serial.println("Beim Versuch den Status der Steckdose auszulesen, ist ein Fehler aufgetreten");
  }
}

bool Fritz::getActorState() {
  try {
    _errorState = false;
    _actorState = api.getSwitchState(fritz_ain);
    return _actorState;
  } catch (int e) {
    _errorState = true;
    Serial.println("Got errorCode during execution " + String(e));
  }
  return false;
}

bool Fritz::startActor() {
  try {
    _errorState = false;
    _actorState = api.setSwitchOn(fritz_ain);
    return _actorState;
  } catch (int e) {
    _errorState = true;
    Serial.println("Got errorCode during execution " + String(e));
  }
  return false;
  
}

bool Fritz::stopActor() {
  try {
    _errorState = false;
    _actorState = api.setSwitchOff(fritz_ain);
    return _actorState;
  } catch (int e) {
    _errorState = true;
    Serial.println("Got errorCode during execution " + String(e));
  }
  return false;
}

void Fritz::print() {
  try {
    double power = api.getSwitchPower(fritz_ain);
    Serial.printf("Current power consumption is %.3f W\n", power);
  } catch (int e) {
    Serial.println("Got errorCode during execution " + String(e));
  }

  try {
    int energy = api.getSwitchEnergy(fritz_ain);
    Serial.printf("Total Energy is %i Wh\n", energy);
  } catch (int e) {
    Serial.println("Got errorCode during execution " + String(e));
  }
  Serial.print("Fritz-Errorstate: ");
  Serial.println(_errorState);
  Serial.print("Fritz-Actorstate: ");
  Serial.println(_actorState);  
}

/**
 * Die eigentliche Pruefung der Werte erfolgt im Arduino. Von diesem
 * kommt hier der Wert des relayStatus an (s.u.).
 * 
 * Hier muss nun entschieden werden, ob ein Schaltvorgang erfolgen soll, oder nicht.
 * 
 * Grundvoraussetzung dafür ist, ob eine Kommunikation mit dem Aktor (==Steckdose)
 * der Fritzbox funktioniert. Dann wäre der Status der Steckdose bekannt, der _errorStatus
 * wäre false (oder 0).
 * 
 * Solange dies nicht der Fall ist, wird keine Aktion durchgeführt und nur immer wieder 
 * versucht, den Status zu lesen.
 * 
 * Gelingt dies, wird geprüft, ob der letzte Schaltvorgang weniger als 5 Minuten her ist.
 * 
 * Ist auch dies der Fall, wird unter Berücksichtigung des nun bekannten _actorState (true == Pumpe hat Strom,
 * false == Steckdose ist aus), versucht den vom Arduino gewünschten relayStatus umzusetzen.
 * 
 * Es heisst im uebrigen hier relayStatus, weil der Arduino selbst noch ein Relais steuert. Ist das
 * Licht am Relay rot, dann ist der relayStatus 1, d.h. die Pumpe sollte stromlos geschaltet werden.
 * 
 * Sind alle Bedingungen erfüllt und kommt es zum Schaltvorgang ( stopActor() oder startActor() werden
 * ausgelöst ), dann wird die Zeit des gerade erfolgten Schaltvorganges vermerkt; damit beginnt nun
 * eine mindestens 5 minütige Pause
 * 
 * Die Schaltzeit sollte im IF-Statement und NICHT in stopActor()/startActor gesetzt werden, 
 * da dort kein wirklicher Schaltvorgang dokumentiert wird (Steckdose könnte schon vorher ohne/mit Strom gewesen sein)
 */
void Fritz::checkSetActor() {

  bool dbg = level == 8;

  if(dbg) Serial.println(F("Aufruf checkSetActor()"));
  
  // doc["rs"]=relayStatus, 0 == keine Limit gerissen, 1 == Pumpe müsste ausgeschaltet werden

  //unsigned long lastActorAction;
  //unsigned long minRelaisActorInterval=300000; //Hoechstens alle 5Minuten

  //Ist noch ein Error vorhanden, dann pruefe ZUERST den Status des Actors
  if(_errorState) {
    Serial.println(F("Fritz-Fehlerstatus entdeckt, wiederhole Anfrage des Actorstatus"));
    getActorState();
  }

  if(!_errorState) {
    //erst pruefen, ob der letzte Schaltversuch ausreichend lange zurueck liegt
    bool switchAllowed = (millis()-lastActorAction) > minRelaisActorInterval;
    if(switchAllowed) {
      if(relayStatus == 1) {
          if(_actorState && stopActor()) { //_actorState == true bedeutet, dass die Pumpensteckdose Strom hat
            lastActorAction = millis();
          }
      } else {
          if(!_actorState && startActor()) { //_actorState == false bedeutet, dass die Pumpensteckdose KEINEN Strom hat
            lastActorAction = millis();
          }
      }
    } else {
      if(dbg) Serial.println(F("Skippe checkSetActor(), wegen minRelaisActorInterval"));      
    }
  } else {
    Serial.println(F("Fritz-Fehlerstatus konnte nicht behoben werden"));
  }
}
