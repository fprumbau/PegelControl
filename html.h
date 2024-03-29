const char changelog[] PROGMEM = R"=====(
<b>Device info</b>: Papa's Pegelmesser
<li>... vieles andere, siehe Github
<li>34: Die vom Arduino empfangenen JSondaten lassen sich nun ueber die Url http://192.178.168.24/data auslesen 
<li>35: Umruestung auf ESP32 Wrover-B
<li>36: Kommandozeile (ueber Serial) integriert, Return fuer Hilfe
<li>37: Rumpf fuer Fritz Aktorensteuerung integriert
<li>38: Serialisiert die ueber Serial2 empfangenen Daten in einen String und kopiert diesen dann in das globale jsonChar
<li>39: Wifi-Daten werden wieder in der Konfig abgelegt
<li>40: Testmodus mit "test on" fuettert Testdaten (data={jsondaten}) statt Daten aus Serial2 ein (testData sollte forher gesetzt werden)
<li>41: Rumpfmethode Fritz::checkvalues() soll ueber die vom Arduino uebermittelten Daten pegel, pegelLimit, temperature, temperatureLimit das Wasserwerk schalten
<li>42: Schalten der Fritzmethode implementiert, Debuglevel schaltbar, Ausgabe ergaenzt.
<li>43: Debug und Debuglevel lassen sich wieder aus der Weboberflaeche setzen.
<li>44: Ein Fix in FritzBox.cpp laesst ein Schaltung des Aktors von Anfang an und nicht erst nach 300000ms zu.
)=====";

#define VERSION 44

const char html[] PROGMEM = R"=====(

<!DOCTYPE html>
<html>
<head>
<title>Zisterne</title>
<meta charset="utf-8"/>
<meta http-equiv='cache-control' content='no-cache'>
<meta http-equiv='expires' content='0'>
<meta http-equiv='pragma' content='no-cache'>

<style media='screen' type='text/css'>
  .date{font-weight:bold;color:#fff;}
  .dbg{font-size:0.7em;margin-top:2px;}
  #console{
    box-shadow: inset 0px 1px 4px #666;overflow-y:scroll;font-size:0.7em;
    width:600px;height:427px;background-color:#505050;border:1px solid #606060;
    color:beige;padding:5px;margin-top:10px;
  }
  #msgDiv {
    background-color:beige;color:red;font-style:bold;width:600px;padding:5px;visibility:hidden;
  }
  div{position: relative;float:left;}
  div2{position: absolute;color:#fed;line-height: 21px;}
  div3{position: relative;height:180px; width:720px;float:left;font-family:Arial,serif;font-weight: bold;font-size:18px;}
  div5{position:absolute;background: rgba(120,90,0,0.4);}
  button{color:#505050;background:beige;font-weight:bold;border:2px solid dardkred;width:120px;height:25px;}
  .outsideWrapper{
      width:360px; height:600px;
      margin:5px 10px;
      border:0px solid blue;
  }
  .insideWrapper{
      width:100%; height:100%;
      position:relative;
  }
  .coveredImage{
      width:100%; height:600px;
      position:absolute; top:0px; left:0px;
  }
  .coveringCanvas{
      width:100%; height:110%;
      position:absolute; top:0px; left:0px;
  }
  .keys {
    font-weight:bold;color:beige;background-color:#505050;padding-left:3px;padding-right:2px;height:16px;
  } 
  .vals {
     color:cyan;font-family:Courier;background-color:#505050;padding-left:3px;padding-right:5px;height:16px;
  }
  .orange { color:orange; }
  .white { color:white; }
</style>
</head>
<body style='background: #000;'>

<div class="outsideWrapper">
    <div class="insideWrapper">
        <img src="/pegel/brunnen.svg" class="coveredImage">
        <canvas id="cc" class="coveringCanvas"></canvas>
    </div>
</div>

<!--onclick="togglePumpe(this.innerHTML);"-->
<div style="width:510px">
    <div style="width:500px;color:white;margin-top:20px;margin-bottom:10px;">
      Relais:&nbsp;&nbsp; <button id="bb">AN</button>&nbsp;<button id="bu" onclick="document.location.href='/update';">Update</button>
      <input id="dbg" class="dbg" onclick="toggleDebug();" type="checkbox">&nbsp;Debug
      <input id="level" type="text" onblur="setLevel();" style="width:30px;" value="0"/> 
      <button onclick="setLevel();" style="width:42px">SET</button>
      <input id="counter" type="text" alt="Laufender Counter" style="width:50px;" value="0"/> 
    </div>
  
    <table cellspacing=0 style="margin-top:10px">
      <tr><td class="keys">Pegel: </td><td class="vals" id="pegel">87cm</td><td class="vals" id="pl">(Limit: 67cm)</td><tr>
      <tr><td class="keys">Temperatur: </td><td class="vals" id="temperature">23°C</td><td class="vals" id="tl">(Limit: 45°C)</td><tr>
      <tr><td class="keys">Luftfeuchte: </td><td class="vals" id="humidity">43%</td><td class="vals"></td><tr>
      <tr><td class="keys">Hysterese: </td><td class="vals"></td><td class="vals" id="hy">2°C / 3cm</td><tr>
    </table>
    
    <div id="console"></div>
    <div id="msgDiv"></div>
</div>
<script id='smain2'>

//Reconnecting-websocket
!function(a,b){"function"==typeof define&&define.amd?define([],b):"undefined"!=typeof module&&module.exports?module.exports=b():a.ReconnectingWebSocket=b()}(this,function(){function a(b,c,d){function l(a,b){var c=document.createEvent("CustomEvent");
return c.initCustomEvent(a,!1,!1,b),c}var e={debug:!1,automaticOpen:!0,reconnectInterval:1e3,maxReconnectInterval:3e4,reconnectDecay:1.5,timeoutInterval:2e3};d||(d={});for(var f in e)this[f]="undefined"!=typeof d[f]?d[f]:e[f];
this.url=b,this.reconnectAttempts=0,this.readyState=WebSocket.CONNECTING,this.protocol=null;var h,g=this,i=!1,j=!1,k=document.createElement("div");k.addEventListener("open",function(a){g.onopen(a)}),k.addEventListener("close",function(a){g.onclose(a)}),
k.addEventListener("connecting",function(a){g.onconnecting(a)}),k.addEventListener("message",function(a){g.onmessage(a)}),k.addEventListener("error",function(a){g.onerror(a)}),this.addEventListener=k.addEventListener.bind(k),this.removeEventListener=k.removeEventListener.bind(k),
this.dispatchEvent=k.dispatchEvent.bind(k),this.open=function(b){h=new WebSocket(g.url,c||[]),b||k.dispatchEvent(l("connecting")),(g.debug||a.debugAll)&&console.debug("ReconnectingWebSocket","attempt-connect",g.url);var d=h,
e=setTimeout(function(){(g.debug||a.debugAll)&&console.debug("ReconnectingWebSocket","connection-timeout",g.url),j=!0,d.close(),j=!1},g.timeoutInterval);h.onopen=function(){clearTimeout(e),(g.debug||a.debugAll)&&console.debug("ReconnectingWebSocket","onopen",g.url),
g.protocol=h.protocol,g.readyState=WebSocket.OPEN,g.reconnectAttempts=0;var d=l("open");d.isReconnect=b,b=!1,k.dispatchEvent(d)},h.onclose=function(c){if(clearTimeout(e),h=null,i)g.readyState=WebSocket.CLOSED,k.dispatchEvent(l("close"));
else{g.readyState=WebSocket.CONNECTING;var d=l("connecting");d.code=c.code,d.reason=c.reason,d.wasClean=c.wasClean,k.dispatchEvent(d),b||j||((g.debug||a.debugAll)&&console.debug("ReconnectingWebSocket","onclose",g.url),k.dispatchEvent(l("close")));
var e=g.reconnectInterval*Math.pow(g.reconnectDecay,g.reconnectAttempts);setTimeout(function(){g.reconnectAttempts++,g.open(!0)},e>g.maxReconnectInterval?g.maxReconnectInterval:e)}},h.onmessage=function(b){(g.debug||a.debugAll)&&console.debug("ReconnectingWebSocket","onmessage",g.url,b.data);
var c=l("message");c.data=b.data,k.dispatchEvent(c)},h.onerror=function(b){(g.debug||a.debugAll)&&console.debug("ReconnectingWebSocket","onerror",g.url,b),k.dispatchEvent(l("error"))}},1==this.automaticOpen&&this.open(!1),this.send=function(b){if(h)
return(g.debug||a.debugAll)&&console.debug("ReconnectingWebSocket","send",g.url,b),h.send(b);throw"INVALID_STATE_ERR : Pausing to reconnect websocket"},this.close=function(a,b){"undefined"==typeof a&&(a=1e3),i=!0,h&&h.close(a,b)},this.refresh=function(){h&&h.close()}}
return a.prototype.onopen=function(){},a.prototype.onclose=function(){},a.prototype.onconnecting=function(){},a.prototype.onmessage=function(){},
a.prototype.onerror=function(){},a.debugAll=!1,a.CONNECTING=WebSocket.CONNECTING,a.OPEN=WebSocket.OPEN,a.CLOSING=WebSocket.CLOSING,a.CLOSED=WebSocket.CLOSED,a});

var ip = location.host;
var connection = new ReconnectingWebSocket('ws://' + ip + '/ws', null, { debug:true, reconnectInterval: 6000, reconnectDecay: 1.1, maxReconnectInterval: 10000 });

var debug = false;

var ctx = document.getElementById('cc').getContext('2d');

function isBlank(str){
    return str === null || str.match(/^ *$/) !== null;
}

function pegel(procent, volume) {
        h = (75 / 100) * procent;
        ctx.clearRect(0, 0, cc.width, cc.height);
        ctx.beginPath();
        //x,y,with,height,
        //height==-1 == minimale Fuellung
        //height==75 == Voll bis zum Konus
        ctx.globalAlpha = 0.7;
        ctx.fillStyle = '#1E22F0';
        ctx.fillRect(37, 133.5, 225, -h)
        ctx.globalAlpha = 1.0;
        ctx.fillStyle = 'yellow';
        ctx.font = 'bold 12px arial'; 
        ctx.fillText(volume+' m³',128,93);        
        ctx.stroke();
}

function setPegel(strVal, strVol) {
        var peg = parseInt(strVal);
        if(debug) log('Parsed pegel value: ' + peg);
        //0 == 0%, 250 == 100%
        var pegPercent = ( peg / 250 ) * 100;
        if(debug) log('Filled ' + pegPercent);
        pegel(pegPercent, strVol);
}

function log(msg) {
        cs = document.getElementById('console');
        cs.innerHTML = cs.innerHTML + '<br>' + msg;
        cs.scrollTop = cs.scrollHeight;
}

console.log('Trying to open Webclient socket');
log('Trying to open Webclient socket');

connection.onopen = function () { 
  connection.send('Connect ' + new Date()); 
  if(null != json) {
     updateUiFromData();
  }
};

var errCount = 0;
connection.onerror = function (error) { 
  errCount++;
  if(errCount>3) {
    console.log('wsServer Error ', error);
    log('wsServer Error ' + error);
    if(!debug) {
       setInterval("document.location.href='/pegel';", 10000);
    }
  }
};

//vom Server empfangen
var json = null;
var server = '';
var data = '';
connection.onmessage = function (e) { 
  errCount = 0;
  if(server == '') {
    server = e.data;
    console.log('Server: ', server);
    log('Server: ' + server);
  } else {
    data = e.data;
    console.log('Daten: ', data);
    if(data[0] == '{') {
      if(debug) {      
        log(data);
      }
      json = JSON.parse(data); 
      updateUiFromData();          
    } else {
       log(data);      
    }
  }
};
console.log('End trying to open Webclient socket');
log('End trying to open webclient socket');

//Keine State-Information hier, die Bestätigung kommt mit Websocket-Datagramm
function togglePumpe(txt) {
    if(txt == 'AN') {
      connection.send("@+"); //Pumpenstrom einschalten
    } else {
      connection.send("@-"); //Pumpenstrom abschalten
    }
}

function toggleDebug() {
    var debug=document.getElementById("dbg").checked;
    connection.send("@d"+debug);
    console.log("toggleDebug(), Debug value: " + debug + "; Send: @d"+debug);
}

function setLevel() {
    var level = document.getElementById("level").value;
    connection.send("@l"+level);
    console.log("setLevel(), Level value: " + level + "; Send: @l" + level);
}

function updateUiFromData() {
      var pegel = json.p;
      document.getElementById('pegel').innerHTML='<b class=white>' + pegel + '</b> cm';
      setPegel(pegel, json.v);       
      document.getElementById('temperature').innerHTML='<b class=white>' + json.t + '</b> °C'; 
      document.getElementById('humidity').innerHTML='<b class=white>' + json.h+'</b> %'; 
      document.getElementById('pl').innerHTML="(Limit: <b class=orange>" + json.PL + '</b> cm)'; 
      document.getElementById('tl').innerHTML="(Limit: <b class=orange>" + json.TL + '</b> °C)'; 
      document.getElementById('hy').innerHTML='<b class=white>' + json.ht + '</b> °C / <b class=white>' + json.hp + '</b> cm'; 
      document.getElementById("dbg").checked=json.d;
      document.getElementById("level").value=json.level;
      document.getElementById("counter").value=json.ct;
      var relayStatus = json.rs;
      var relButton = document.getElementById('bb');
      if(relayStatus == '1') {
        relButton.innerHTML="AUS";
        relButton.style='color:yellow;background-color:red;'        
      } else {
        relButton.innerHTML="AN";
        relButton.style='color:#505050;background-color:beige;';
      }
      var msg = json.m;
      var msgDiv = document.getElementById('msgDiv');
      if(!isBlank(msg)){
        if(msg.charAt(0)=='@' && msg.length > 2) { //Nachrichten, die mit '_' beginnen, sind Statusmeldungen
          msgDiv.style.visibility='visible';
          msgDiv.innerHTML=msg.substring(2);
        } else {
          log(msg);
        }       
      } else {
        msgDiv.style.visibility='hidden';
      } 
}

//anfangs sollte Debug nicht selektiert sein
document.getElementById("dbg").checked=false;
</script>
</body>
</html>

)=====";

const char update[] PROGMEM = R"=====(
<!DOCTYPE html><html lang="en" style="height:100%;"><head>
<meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=no"/>
<title>{title}</title>
<style>
.c{text-align: center;} 
div,input{padding:5px;font-size:1em;} 
input{width:95%;margin-bottom:5px;border-radius: 4px;} 
body{
  height:100%;
  text-align: center;
  font-family:verdana;
  background-image:url("data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAAcAAAAHCAYAAADEUlfTAAAEfnpUWHRSYXcgcHJvZmlsZSB0eXBlIGV4aWYAAHjarVdttuMsCP7PKmYJAcSP5WiM57w7eJc/j8YmbW9vpp078SRYQEQeREvb//81+oVH2C3kLESfvF/wuOSSZHTisj875cWN7/6ESfmRT4dAwFJQ3X/6bepn8O0cENzkl0c+hXXaidPQbeZpUPvMgs7Ui9OQys7n+ZvSHJfd3XLmu27DxMLT6PNvFxCMamCqkGwKPr5jFoUHGjWDGr6sviuBZnVonZ9ex46O7lPwjt5T7JY8+foYClr8VPBPMZp8ttexGxG694jPmR8Ezg48v8SutRpb2/bVZecRKU9zUbeljB4UC0zpGObRAl5DP4yW0CKWuAKxCjQL2kqcWBDtxo4rZ268DbryChedbBJARVZEvPOiBkmyDlB6+B03CZq0EjASXYGagi2HLzzmTWO+lSNmrgxNYRhjjPjS6BXzb9phqLWeusxLPGIFv6QnINzoyPUvtAAItxlTG/Edje7yZrkDVoGgjTBHLDAvZTdRjM/c0oGzQs8WR8u+NTjUaQAhwtwGZ1iBwOJZjT0vQSQwI44R+GR4LuqkAAE2k8rUgI1iJwSJ0ufGmMBDV0x2NkoLgDD1GgBN0gywnDPkT3AROZRNzZGZeQsWLVn26p03733wvUbloMEFCz6EEEMKOWp00aKPIcaYYk6SFCXMkk+BUkwp5YxJM0xnjM7QyLlI0eKKFV9CiSWVvCJ9Vrfa6tewxjWtuUrViu1ffQ1UY001b7whlTa32ea3sMUtbbkh15o216z5FlpsqeUDtYnqI2r8hNw1ajxR64i5oRdO1MAO4WaCezmxjhkQE8dAPHQEkNDSMVsiOycduY7ZkgSbwgSosXVwKnfEgKDbWKzxgd2J3CVuZO4j3OQ75KhD9y+Qow7dRO4rbi9Qq3mcKDoA6ruwx3TRhsLWHCIRs5S41qV3YtdF2ssC95aDdU3pXUXQtGAdwq9V6IHxsR8npZ/5cVJCx3AGAjufModcNaO0leIq48WP3O8BVZ3Hu2X5TkiX0g+EdCEFuFFdy8iiHjVf2Vm/M9grSuggdQrG+9wsB+sXjC76kNKlQvX7FG+4RPwnlwoUkutl+bJD38nEcA6MjMdhv/sUTLnlCrxhP/WZuv7s0Jg1HjkRZHilNhPhtGVjQ3+bTXSRiYLKICmeoRgO6ZZiq74j+xxsqf/ALfrTBnnXLboh9FO36K368YZb3SM5ZmP/ZON9t+j9snbtFt3C9VO36KMyeOEWPaD4A7fo8+r82i36mlwXblkvE6h0MsvE/abFQYmzeR/kbqM/6CTpXQ/4T6N73HBW1w/8u5WxxwpZYLN8V5WvqBh9V8Ks9BKmuxduedqSp41cqk+4RWCLlNTdzLie4BgZ0QIoYGrop2KAeq2rj/1duDrTAMEXIV1KXwn7rXlTl0qseielKR5LYQhx5Uu6F+7rUD/dEyj+xYXhVWrSLezcU12PKJ4Fzg4cJu3g4lLp77fJXmofGO9RN3aYv8sc6ndO3Fxu2XQGRXHNw19z+g0EzfGFJDrJyAAAAAZiS0dEAP8A/wD/oL2nkwAAAAlwSFlzAAAOwwAADsMBx2+oZAAAAAd0SU1FB+IFEAwsCt9fsxIAAABDSURBVAjXfc67DYBADATRMUf/hVAGCU1ZHgInp+Oz0kZPtjau8zAiWKOyA4wxHpiZjQDztXY3PuMf0m/VZcyEVfWKN854GDbTdSsxAAAAAElFTkSuQmCC");} button{border:0;background-color:#b2ad98;color:#545147;line-height:2.4rem;font-size:1.2rem;width:100%;} #header{height:120px;text-align: left;color:#cbc6af;background-image:url("data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAABmJLR0QA/wD/AP+gvaeTAAAACXBIWXMAAA7DAAAOwwHHb6hkAAAAB3RJTUUH4gUQDDYwqX6QewAAAA1JREFUCNdjMDXWOwMAApoBY44cvvsAAAAASUVORK5CYII=");}#footer{height:24px;color:#292723;background-image:url("data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAABmJLR0QA/wD/AP+gvaeTAAAACXBIWXMAAA7DAAAOwwHHb6hkAAAAB3RJTUUH4gUQDDg61yhU6wAAAA1JREFUCNdjMDXWSwMAAjQA/b8+kbgAAAAASUVORK5CYII=");
}
#body{text-align:left;display:inline-block;min-width:260px;min-height: calc(100% - 186px);
color:#545147;}h2{margin-top: 4px;margin-left:10px;}p{margin-top: 2px;margin-bottom: 0px;margin-left: 10px;}
</style>
<script>var redirect = false;
{redirect};
if(redirect) {
  setTimeout('document.location.href="/update";', 3000);
}
</script>
</head>
<body>
<div id="header">
<h2>{banner}</h2>
<p>{build}</p><p>{branch}</p><p>{deviceInfo}</p></div> 
<div id="body"><form method='POST' action='' enctype='multipart/form-data'><input type='file' name='update'><br/>
<br/><input type='submit' value='Update'></form><br>{footer}</div><div id="footer"><p>Pegelmesser</p>
</div> </body></html>
)=====";
