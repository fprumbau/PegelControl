#include "OTA.h"

void OTA::init(ESP8266WebServer& server, const char* host, const char* update_path, const char* update_username, const char* update_password, int FW_VERSION) {
  //OTA is possible only with 4mb memory
  long flashSize = ESP.getFlashChipSize();
  Serial.println("");
  Serial.print("Flash Size: ");
  Serial.println(flashSize);
  if(flashSize > 4000000) {
     //set web UI
     MDNS.begin(host);
     MDNS.addService("http", "tcp", 80);
     String vrs = "Build: ";
     vrs+=FW_VERSION;
     Serial.printf("\n\nHTTPUpdateServer ready! Open http://%s.local/%s in your browser\n", host, update_path);
     _otaUpdater.setUpdaterUi("Title", "Banner", vrs, "Branch : master","Device info : ESP8266","footer");
     _otaUpdater.setup(&server,update_path, update_username, update_password);
  } else {
     Serial.println("Flash OTA programming only possible with 4Mb Flash size!!!");
  }
}

