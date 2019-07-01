#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <time.h>
#include <WebSocketsServer.h>
#include <FS.h>  
#include "MyWifi.h"
#include "webpage.h"
#include "OTA.h"
#include "Log.h"
#include "config.h"

#define IFSET(var,pos) (((var)>>(pos)) & 1)

int timezone = 2;
int dst = 0;

//Ringspeicher, Groesse in Log.h definiert
LOG logs;

const int FW_VERSION = 31;

const char* update_path = "/update";
const char* update_username = "admin";
const char* update_password = "admin";

OTA ota;
CFG config;
MyWifi myWifi;
ESP8266WebServer server(80);
WebSocketsServer wsServer = WebSocketsServer(81);

const char* host = "esp8266C";

//client connected to send?
volatile bool ready = false;

uint8_t clientCount=0;
uint8_t clients[256] = {-1};
bool notifiedNoClient = false;

bool debug = false;
int level = 0;

//weil der esp8266 nur ein UART fuer RX/TX hat, das zweite hat nur TX
//SoftwareSerial mySerial(13, 15, false, 256); //Rx,Tx auf Pins 13 + 15 ( D7 + D8 )
SoftwareSerial mySerial(5, 4, false, 256); //Rx,Tx auf Pins 5 + 4 ( D1 + D2  )

//D7(13)[RX] -> Arduino D11[TX]
//D8(15)[TX] -> Arduino D10[RX]

unsigned long wsServerLastSend = -1;

/*
 * Schreibt die Webseite in Teilen (<6kb)
 * auf den Webclient, der sich gerade verbunden
 * hat. Es ist wichtig, hier die korrekte
 * Laenge des contents zu senden. Weitere Teile
 * sollten immer mit server.sendContent_P(partN)
 * versendet werden. Das _P ist hier wichtig, da
 * die Seitendefinition im PROGMEM liegen (s. webpage.h)
 */
void webpage();


/**
 * Sende Daten zu allen über Websockets verbundenen
 * Clients. Alles, was NICHT SBMS-Daten sind, also
 * Fehler- bzw. Statusmeldungen MUSS mit einem '@'
 * eingeleitet werden, sonst wird es von der Webseite
 * falsch interpretiert und führt zu wilden Werten
 * z.B. beim Batteriestatus.
 * 
 * wichtig: auch sehr kurz aufeinanderfolgende
 *          Nachrichten werden versendet, sonst max.
 *          alle 100ms
 */
void sendClients(String msg, bool wichtig);

/**
 * Toggle: 
 * - @d1-true
 * - @d1-false
 * - @d2-true
 * - @d2-false
 * - @l1-10 Level setzen
 */
void toggleDebug(unsigned char* payload);

void handleRoot();
 
void handleWebRequests();

String loadMsg;

/**
 * Initialisierung
 */
void setup() {
  
  Serial.begin(115200);  //USB
  mySerial.begin(19200); //Arduino

  //Initialize File System
  SPIFFS.begin();

  //config._ssid="";
  //config._password="";
  //config.save();
  
  //lese SPIFFS config file system
  config.init(myWifi);

  // etabliere Wifi Verbindung
  myWifi.connect();

  // start WebsocketServer server
  wsServer.onEvent(webSocketEvent);
  wsServer.begin();   

  // start Webserver
  server.on("/",handleRoot);
  server.onNotFound(handleWebRequests);
  server.on("/pegel", webpage);

  Serial.print("\nStarting firmware version... ");
  Serial.println(FW_VERSION);

  // initialize other the air updates
  ota.init(server, host, update_path, update_username, update_password, FW_VERSION);
  
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
}

/**
 * Standardloop
 */
void loop() {
  yield();
  wsServer.loop();
  yield();
  server.handleClient(); 
  yield();
  if(mySerial.available()) {
    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(mySerial);
    if(debug) {
      root.prettyPrintTo(Serial);
    }
    //
    bool wichtig = false;
    String msg = root["m"];
    if(msg.startsWith("*")) {
            
      //Abspeichern, wichtige Statusnachricht
      time_t now = time(nullptr);
      String localTime = String(ctime(&now));
      msg = "<b class=date>" + localTime + "</b>&nbsp;&nbsp;" + msg.substring(1);
      root["m"]=msg;
      
      logs.append(msg);
      logs.print();
      
      String saveDbgMsg = logs.save();
      if(IFSET(level,3)) {
        sendClients(saveDbgMsg, true);
      }
      wichtig = true;
    } 
    root["level"]=level;
    //da 2 SoftwareSerials benoetigt werden, kann NICHT ueber 2 gleichzeitig gelesen
    //werden, darum wird der Debug-Wert aus dem ESP zum Client uebermittelt
    root["d"]=debug;
    char jsonChar[512];
    root.printTo(jsonChar);
    sendClients(jsonChar, wichtig);
  }
}

/*
 * Schreibt die Webseite in Teilen (<6kb)
 * auf den Webclient, der sich gerade verbunden
 * hat. Es ist wichtig, hier die korrekte
 * Laenge des contents zu senden. Weitere Teile
 * sollten immer mit server.sendContent_P(partN)
 * versendet werden. Das _P ist hier wichtig, da
 * die Seitendefinition im PROGMEM liegen (s. webpage.h)
 */
void webpage() {
  long s1 = sizeof(part1);
  long s2 = sizeof(part2);

  String connStr = "var connection = new WebSocket('ws://";
  connStr+=myWifi.getIpAddress();
  connStr+=":81/', ['arduino']);";
  int s3 = connStr.length();
  
  long totalSize = s1 + s2 + s3;
  if(debug) {
    Serial.print("\np1: ");
    Serial.println(s1);
    Serial.print("p2: ");
    Serial.println(s2);
    Serial.print("total: ");
    Serial.println(totalSize);
    Serial.println("");
  }
  server.setContentLength(totalSize);
  server.send_P(200, "text/html", part1);
  server.sendContent(connStr);
  server.sendContent_P(part2);
}

void sendClients(String msg, bool wichtig) {

  if(msg.equals("{}")){
    return;
  }        
  if(IFSET(level,2)) {
    Serial.println(msg);
  }
  
  if(clientCount<=0) {
    if(debug) {
      Serial.println("Clientcount ist 0, sende nichts");
    }
    return;
  }
  if(debug) {
    Serial.println("sendClients got called");
  }
  if(!wichtig) {
    if(wsServerLastSend>0 && (millis()-wsServerLastSend) < 100) {
      if(debug) {
        Serial.print("Could not send data multiple times in 100ms; disgarding ");
      }
      return;
    }
    wsServerLastSend = millis();
  }
  for(int m=0; m<clientCount; m++) {
     uint8_t client = clients[m];
     if(debug) {
       Serial.printf("Sending client %u ( %u ) from %u clients\n", (m+1), client, clientCount);
     }
     wsServer.sendTXT(client, msg);
  }
}

/**
 * Websocket-Events, wenn neue Clients sich verbinden, wenn die clients
 * selbst senden oder wenn sie geschlossen werden.
 */
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

    switch(type) {
        case WStype_DISCONNECTED: {
            //Client 'num' aus Liste rausnehmen
            uint8_t newClients[256];
            for(int a=0; a<256; a++) {
              newClients[a] = clients[a];
            }
            int c=0;
            for(int x=0; x < clientCount; x++) {
              if(num != newClients[x]) {
                clients[c] = newClients[x];
                c++;
              } else {
                clientCount--;
              }
            }
            if(clientCount == 0) {
              notifiedNoClient = false;
              ready = false;
            }
            Serial.printf("[%u] Disconnected! Remaining %u\n", num, clientCount);
            break; }
        case WStype_CONNECTED: {
            IPAddress ip = wsServer.remoteIP(num);
            Serial.println("");            

            // send message to client
            wsServer.sendTXT(num, "Connected<br>");

            bool alreadyListed = false;
            int y = 0;
            for(; y < clientCount; y++) {
              if(num == clients[y]) {
                alreadyListed = true;
                break;
              }
            }
            if(!alreadyListed) {
              clients[y]=num;
              clientCount++;
            }
            Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s; ConnCount: %u\n", num, ip[0], ip[1], ip[2], ip[3], payload, clientCount);
            ready = true;

            //Dem neuen Client alle vorhandenen logEvents aus logs senden!!!            
            if(IFSET(level,3)) {      
                sendClients("Loadmessage folgend: ", true);        
                sendClients(loadMsg, true); //TEST!!!!
            }     
            for(int le = 0; le < logs.length(); le++) {               
                String msg = logs.get(le);
                if(msg == "") {
                  continue;          
                }
                wsServer.sendTXT(num, msg);
            }    
            //Arduino sendet neue daten nur, wenn eine Aenderung vorliegt, darum fordere Daten an!
            Serial.printf("[Client %u] Initiating datarequest for new client: @datarequest\n", num);
            mySerial.print("@datarequest");   
            break; }
        case WStype_TEXT:
            Serial.printf("[Client %u] received: %s\n", num, payload);

            if(payload[0] == '@') {         
              if(payload[1] == 'd' || payload[1] == 'l') {                
                 toggleDebug(payload);
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
        mySerial.print("d1"); 
        logs.setDebug(true);
    } else { //FALSE
        msg = "Switched debug to false";
        debug = false;
        mySerial.print("d0"); //Arduino kennt nur debug bzw. debug1
        logs.setDebug(false);
    }  
  } else if(payload[1] == 'l') {
    String pl = String((char*)payload);
    pl = pl.substring(2);
    Serial.println(pl);
    level = pl.toInt();

    if(IFSET(level,1)) {
      msg = "Debugging level 1 activated";
    }
    if(IFSET(level,2)) {
      msg = "Debugging level 2 activated";
    }
    if(IFSET(level,3)) {
      msg = "Debugging level 3 activated: Logs";
    }
    if(IFSET(level,4)) {
      msg = "Debugging level 4 activated";      
    }  
    sendClients(msg, true);  
  }
}

bool loadFromSpiffs(String path){
  String dataType = "text/plain";
  if(path.endsWith("/")) path += "pegel";
 
  if(path.endsWith(".src")) path = path.substring(0, path.lastIndexOf("."));
  else if(path.endsWith(".html")) dataType = "text/html";
  else if(path.endsWith(".htm")) dataType = "text/html";
  else if(path.endsWith(".css")) dataType = "text/css";
  else if(path.endsWith(".js")) dataType = "application/javascript";
  else if(path.endsWith(".png")) dataType = "image/png";
  else if(path.endsWith(".gif")) dataType = "image/gif";
  else if(path.endsWith(".jpg")) dataType = "image/jpeg";
  else if(path.endsWith(".ico")) dataType = "image/x-icon";
  else if(path.endsWith(".xml")) dataType = "text/xml";
  else if(path.endsWith(".pdf")) dataType = "application/pdf";
  else if(path.endsWith(".zip")) dataType = "application/zip";
  else if(path.endsWith(".svg")) dataType = "image/svg+xml";

  if(path.startsWith("/pegel")) {
    path = path.substring(6);
  }
  if(debug) {
    String loading = "Loading ";
    loading+=path.c_str();
    sendClients(loading, true);
  }
  
  File dataFile = SPIFFS.open(path.c_str(), "r");
  if (server.hasArg("download")) dataType = "application/octet-stream";
  if (server.streamFile(dataFile, dataType) != dataFile.size()) {
  }
 
  dataFile.close();
  return true;
}

void handleRoot(){
  server.sendHeader("Location", "/pegel",true);   //Redirect to our html web page
  server.send(302, "text/plane","");
}
 
void handleWebRequests(){
  if(loadFromSpiffs(server.uri())) return;
  String message = "File Not Detected\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " NAME:"+server.argName(i) + "\n VALUE:" + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  Serial.println(message);
}
