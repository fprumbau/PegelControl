#include "config.h"
#include "MyWifi.h"

void CFG::init(MyWifi& myWifi) {
    if(!load()) {
      Serial.println("Failed to load config");
    } else  {
      Serial.println("Config loaded");
      myWifi._ssid = _ssid;
      myWifi._password = _password;
      Serial.print("Wifi SSID from config: ");
      Serial.println(myWifi._ssid);
    }

    /*if(!save()) {
      Serial.println("Failed to save config");
    } else  {
      Serial.println("Config saved");
    }*/
}

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
  // to be mutble. if you don't use ArduonJson, you may as well use
  // configFile.readString instead
  configFile.readBytes(buf.get(), size);
  Serial.println(buf.get());
  

  StaticJsonDocument<1024> doc;
  //JsonObject& json = jsonBuffer.parseObject(buf.get());
  auto error = deserializeJson(doc, buf.get());

  if(error) {
    Serial.println("Failed to parse config file");
    return false;
  }

  _ssid = doc["ssid"];
  _password = doc["password"];
  
  Serial.print("SSID from config: ");
  Serial.println(_ssid);
  Serial.print("Password from config: ");
  Serial.println(_password);

  return true;
}

bool CFG::save() {
  if(!SPIFFS.begin()) {
    Serial.println("Failed to mount file system (save)");
    return false;
  }

  StaticJsonDocument<1024> doc;
  
  //doc["socLimit"] = String(_socLimit);

  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) {
    Serial.println("Failed to open config file for writing");
    return false;
  }

  //json.printTo(configFile);
  serializeJson(doc, configFile);
  return true;
}
