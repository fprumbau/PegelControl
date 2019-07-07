#include "html.h"
#include "global.h"

/**
 * Changelog:
 * 
 * FW_VERSION 33: ArduionoJson 6, Esp8266 v.2.5
 * 
 */

int timezone = 2;
int dst = 0;

const int FW_VERSION = 35;

const char* update_path = "/update";
const char* update_username = "admin";
const char* update_password = "admin";

const char* host = "pegel";

//client connected to send?
volatile bool ready = false;

//D7(13)[RX] -> Arduino D11[TX]
//D8(15)[TX] -> Arduino D10[RX]

//Anzeige Wasserstand/Pegel in Jsonseite
char jsonChar[512];

/**
 * Toggle: 
 * - @d1-true
 * - @d1-false
 * - @d2-true
 * - @d2-false
 * - @l1-10 Level setzen
 */
void toggleDebug(unsigned char* payload);

String loadMsg;

void sendClients(String msg) { 
  ws.textAll(msg.c_str());
}

/**
 * Initialisierung
 */
void setup() {
  
  Serial.begin(115200);  //USB
  Serial2.begin(19200,SERIAL_8N1, 25, 26);

  Serial.println("Starting...");

  SPIFFS.begin();

  //Hier kann die Konfig erstmals initialisiert werden
  myWifi._ssid="Perry";
  myWifi._password="5785340536886787";
  //config.save();
  
  //lese SPIFFS config file system  
  //config.load();

  // etabliere Wifi Verbindung
  myWifi.connect();

  // start Webserver
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->redirect("/pegel");
  });

  server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/favicon.ico", "image/x-icon");
  });

  server.on("/pegel/brunnen.svg", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/brunnen.svg", "image/svg+xml");
  });

  server.on("/pegel", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", html);    
  });

  server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request){
    String json = String(jsonChar);
    if(debug) {
      Serial.print("JsonDATA:");
      Serial.println(json);
    }    
    request->send(200, "text/html", json);    
  });
  
  // start WebsocketServer server
  ws.onEvent(onWsEvent); 

  server.addHandler(&ws);
  server.begin();

  Serial.print("\nStarting firmware version... ");
  Serial.println(VERSION);

  // initialize other the air updates
  updater.init(hostName);
  
  server.begin();

  //Laden evtl. vorhandener Logevents
  loadMsg = logs.load();
  
  logs.print();

  //Zeit
  configTime(timezone * 3600, dst * 0, "pool.ntp.org", "time.nist.gov");
  while (!time(nullptr)) {
    Serial.print(".");
  }
  delay(1000);
  time_t now = time(nullptr);
  Serial.println(ctime(&now));

  Serial.println("Initialisiere Fritz-Api");
  try {
    fritz.init();
    Serial.println("Fritz connected");
  } catch (int e) {
    Serial.println("Could not connect to fritzbox: " + String(e));
  }
}

/**
 * Standardloop
 */
void loop() {

  if(!updater.stopForOTA) {
    commandLine();    //Pruefen, ob eine Kommandozeileneingabe vorhanden ist
    yield();

    DynamicJsonDocument doc(512);
    bool hasData = false;
    if(test) {
      hasData = true;     
      deserializeJson(doc, testData);
      delay(2000); //ERROR: Too many messages queued
    } else {
      hasData = Serial2.available();
      if(hasData) {
        deserializeJson(doc, Serial2);
      }
    }
    
    yield();
    
    if(hasData) {   
         
      if(debug) {
        serializeJsonPretty(doc, Serial);
      }
     
      String msg = doc["m"];
      if(msg.startsWith("*")) {
              
        //Abspeichern, wichtige Statusnachricht
        time_t now = time(nullptr);
        String localTime = String(ctime(&now));
        msg = "<b class=date>" + localTime + "</b>&nbsp;&nbsp;" + msg.substring(1);
        doc["m"]=msg;
        
        logs.append(msg);
        logs.print();
        
        String saveDbgMsg = logs.save();
        if(level == 3) {
          sendClients(saveDbgMsg);
        }
      } 
      //TODO warum???
      doc["level"]=level;
      doc["d"]=debug;

      //Damit eine Abschaltung der Steckdose des Hauswasserwerks entschieden werden kann, werden hier die wichtigsten Werte vermerkt
      relayStatus = doc["rs"]; //0|1, mit 0 == OK, 1 == Fehler
      pegel = doc["p"];
      temperature = doc["t"];
      pegelLimit = doc["PL"];
      temperatureLimit = doc["TL"];      
      
      String json;
      serializeJson(doc, json);
      strcpy(jsonChar, json.c_str()); //Kopiert den Inhalt des json-Strings in das Array jsonChar und haengt ein \0 an
      if(debug) {
          Serial.println(String(jsonChar));
      }
      sendClients(String(jsonChar));
      yield();

      //ggfls. Steckdose schalten
      fritz.checkSetActor();
    }        
  }
  
  //Restart erforderlich, wird durch updater-Objekt nach Upload einer neuen Firmware geregelt
  if(updater.restartRequired) {
    delay(2000);
    ESP.restart();
  }
}

/**
 * Registriere Eventhandler für WebSocketEvents in WebCom
 */
void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
    
    switch (type) {
    case WS_EVT_CONNECT: {       
       
          client->text("@ Connected");

          //Dem neuen Client alle vorhandenen logEvents aus logs senden!!!            
          if(level==3) {      
              sendClients("Loadmessage folgend: ");        
              sendClients(loadMsg); //TEST!!!!
          }     
          for(int le = 0; le < logs.length(); le++) {               
              String msg = logs.get(le);
              if(msg == "") {
                continue;          
              }
              client->text(msg);
          }           
                   
          break;
      }    
      case WS_EVT_DATA:
      
        if(data[0] == '@') {         
          if(data[1] == 'd' || data[1] == 'l') {                
             toggleDebug(data);
          }              
        }
        break;
    }
  
}

/**
 * @d1=true
 * @d1=false
 * @d2=true
 * @d2=true
 * @l1 set Level1-9
 */
void toggleDebug(unsigned char* payload) {
  String msg;
  bool dbgVar;

  //TRUE
  if(payload[1] == 'd') {
    if(payload[2] == 't') {
        msg = "Switched debug to true";
        debug = true;
        Serial2.print("d1"); 
        logs.setDebug(true);
    } else { //FALSE
        msg = "Switched debug to false";
        debug = false;
        Serial2.print("d0"); //Arduino kennt nur debug bzw. debug1
        logs.setDebug(false);
    }  
  } else if(payload[1] == 'l') {
    String pl = String((char*)payload);
    pl = pl.substring(2);
    Serial.println(pl);
    pl.trim();
    level = atoi(pl.c_str());
    msg = "Debuglevel activated: ";
    msg+=level; 
    sendClients(msg);  
  }
}

void commandLine() {
  if(Serial.available()) {
      String cmd = Serial.readString();
      Serial.print("Echo: ");
      Serial.println(cmd);
      String msg = "-";
      if(cmd.startsWith("restart wifi")) {      
        myWifi.reconnect();
      } else if(cmd.startsWith("restart esp")) {      
        msg = F("Restarting ESP...");
        Serial.println(msg);
        sendClients(msg);
        ESP.restart();
      } else if(cmd.startsWith("data ")) {      
        msg = F("Setze Testdaten");    
        //{"p":37,"PL":30,"ct":28760,"v":"0.65","t":19,"TL":40,"h":93,"rs":0,"d":true,"ht":2,"hp":3,"m":"","level":0} 
        testData = cmd.substring(5); 
        msg+=testData;     
      } else if(cmd.startsWith("debug on")) {        
        debug = true;
      } else if(cmd.startsWith("debug off")) {         
        debug = false;
      } else if(cmd.startsWith("test on")) {        
        test = true;
      } else if(cmd.startsWith("test off")) {         
        test = false;
      } else if(cmd.startsWith("actor start")) {        
        Serial.println(fritz.startActor());
      } else if(cmd.startsWith("actor stop")) {         
        Serial.println(fritz.stopActor());
      } else if(cmd.startsWith("print")) {         
        fritz.print();
        Serial.print("Relaystatus: ");
        Serial.println(relayStatus);
        Serial.print("lastActorAction: ");
        Serial.println(lastActorAction);        
        Serial.print("Level: ");
        Serial.println(level);   
      } else if(cmd.startsWith("level ")) {              
        String lv = cmd.substring(6); 
        level = atoi(lv.c_str());
        msg = "Debuglevel: ";
        msg+=level;
      } else {
        Serial.println(F("Available commands:"));
        Serial.println(F(" - restart wifi  :: restarting Wifi connection"));
        Serial.println(F(" - restart esp   :: restarting whole ESP32"));
        Serial.println(F(" - debug  on|off :: enable/disable debug"));  
        Serial.println(F(" - level  LEVEL :: Debuglevel schalten, 0-9"));  
        Serial.println(F(" - actor start|stop   :: Fritz Steckdose schalten"));     
        Serial.println(F(" - test  on|off :: Testmodus aktivieren/deaktivieren"));
        Serial.println(F(" - data  TESTDATA :: Testdaten setzen"));
        Serial.println(F(" - print :: Schreibe einige abgeleitete Werte auf den Bildschirm"));
        return;
      }
      if(!msg.equals("-") && !msg.equals("")) {
        Serial.println(msg);
        sendClients(msg);
      }
    }  
}
