#include "FritzBox.h"

#include <ArduinoFritzApi.h>

#include "global.h"

void Fritz::init() {
  //api = FritzApi(fritz_user, fritz_password, fritz_ip)&; 
  api.init();
}

bool Fritz::startActor() {
  try {
    return api.setSwitchOn(fritz_ain);
  } catch (int e) {
    Serial.println("Got errorCode during execution " + String(e));
  }
  return false;
  
}

bool Fritz::stopActor() {
  try {
    return api.setSwitchOff(fritz_ain);
  } catch (int e) {
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
}

void Fritz::checkValues() {
  //TODO pruefen von pegel,pegelLimit,temperature,temperatureLimit und damit Schalten der Steckdose des Wasserwerks
  Serial.println("Fritz::checkValues() => TODO ");
}
