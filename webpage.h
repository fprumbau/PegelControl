
const char part1[] PROGMEM = R"=====(

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

)=====";

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
const char part2[] PROGMEM = R"=====(

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
