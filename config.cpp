#include "config.h"
#include "MyWifi.h"

#include "global.h"

bool CFG::load() {
  Serial.println("Mounting FS...");
  if(!SPIFFS.begin()) {
    Serial.println("Failed to mount file system (read)");
    return false;
  }
  File configFile = SPIFFS.open("/config.json", "r");
  if(!configFile){
    Serial.println("Failed to open config file");
    return false;
  }
  size_t size = configFile.size();
  Serial.print("size ");
  Serial.println(size);
  if(size>1024) {
     Serial.println("Config file is to large");
     return false;
  }
  //allocate a buffer to store contetns of the file.
  std::unique_ptr<char[]> buf(new char[size]);

  //We don't use String heire because ArduinoJson lib req the intput buffer
  // to be mutable. if you don't use ArduonJson, you may as well use configFile.readString instead
  configFile.readBytes(buf.get(), size);
  Serial.println(buf.get());
  
  StaticJsonDocument<1024> doc;
  auto error = deserializeJson(doc, buf.get());

  if(error) {
    Serial.println("Failed to parse config file");
    return false;
  }

  myWifi._ssid = doc["ssid"];
  myWifi._password = doc["password"];
  
  Serial.print("SSID from config: ");
  Serial.println(myWifi._ssid);
  Serial.print("Password from config: ");
  Serial.println(myWifi._password);

  return true;
}

bool CFG::save() {
  if(!SPIFFS.begin()) {
    Serial.println("Failed to mount file system (save)");
    return false;
  }

  StaticJsonDocument<1024> doc;
  
  doc["ssid"] = myWifi._ssid;
  doc["password"] = myWifi._password;
  
  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) {
    Serial.println("Failed to open config file for writing");
    return false;
  }

  serializeJson(doc, configFile);
  return true;
}
