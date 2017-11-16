#include "esp_wpa2.h"
#include <WiFi.h>
#include <WebSocketsServer.h>
#include <DallasTemperature.h>

// SSID to connect to
static const char* ssid = "MMBBS-Intern";
//static const char* ssid = "FRITZ!Box Fon WLAN 7390";
// Username for authentification
static const char* username = "tuttas";
// Password for authentication
static const char* password = "geheim!";
#define EAP_ID "tuttas"
#define EAP_USERNAME "tuttas"
#define EAP_PASSWORD "geheim"

// Data wire is plugged into pin D1 on the ESP8266 12-E - GPIO 5
#define ONE_WIRE_BUS 5
// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature DS18B20(&oneWire);
char temperatureCString[6];
float tempC = 0.0;
float oldTempC = 0.0;
unsigned long timer = 0;


WiFiServer server(80);
WebSocketsServer webSocket = WebSocketsServer(8080);
const String header_ok = "HTTP/1.1 200 OK\r\n";
const String htmlhead = "\r\n<!DOCTYPE HTML>\r\n<html><head><title>ESP32</title> <meta charset=\"UTF-8\"> <link rel=\"stylesheet\" href=\"http://maxcdn.bootstrapcdn.com/bootstrap/3.3.5/css/bootstrap.min.css\"> <script src=\"https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js\"></script></head><body>";
const String htmltail = "\r\n</body></html>\r\n";
const String contentTypeText = "Content-Type: text/html\r\n";
bool flasher = false;
bool state = false;
char buff[20];
String ip;

String getJson() {
  return String("{\"temperature\": " + String(temperatureCString) + ", \"state\":" + String(state) + " }");
}


void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) {

  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] WS Disconnected!\n", num);
      break;
    case WStype_CONNECTED:
      {
        //IPAddress ip = webSocket.remoteIP(num);
        //Serial.printf("[%u] WS Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
        // send message to client
        String s = getJson();
        webSocket.sendTXT(num, s);
      }
      break;
    case WStype_TEXT:
      Serial.printf("[%u] WS get Text: %s\n", num, payload);

      // send message to client
      // webSocket.sendTXT(num, "message here");

      // send data to all connected clients
      // webSocket.broadcastTXT("message here");
      break;
    case WStype_BIN:
      Serial.printf("[%u] WS get binary lenght: %u\n", num, lenght);
      //hexdump(payload, lenght);

      // send message to client
      // webSocket.sendBIN(num, payload, lenght);
      break;
  }
}

void setup() {
  timer = millis();
  // put your setup code here, to run once:
  Serial.begin(115200);

  delay(500);
  Serial.write("\r\nStarte OneWireBus");
  dtostrf(tempC, 2, 2, temperatureCString);
  DS18B20.begin();
  Serial.write("\r\n\r\nSet Output Pin");
  pinMode(4, OUTPUT);


 
   // WPA2 enterprise magic starts here
   
    WiFi.disconnect(true);      
    esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)EAP_ID, strlen(EAP_ID));
    esp_wifi_sta_wpa2_ent_set_username((uint8_t *)EAP_USERNAME, strlen(EAP_USERNAME));
    esp_wifi_sta_wpa2_ent_set_password((uint8_t *)EAP_PASSWORD, strlen(EAP_PASSWORD));
    esp_wpa2_config_t config = WPA2_CONFIG_INIT_DEFAULT(); 
    esp_wifi_sta_wpa2_ent_enable(&config);
    WiFi.begin(ssid);
    // WPA2 enterprise magic ends here
 

  // Normal Connection starts here
  /*
  WiFi.mode(WIFI_STA);
  Serial.write("\r\nConnect to WLAN");
  WiFi.begin(ssid, password);
  */
  // Normal Connection ends here
  

  // Wait for connection AND IP address from DHCP
  Serial.println();
  Serial.println("Waiting for connection and IP Address from DHCP");
  while (WiFi.status() != WL_CONNECTED) {
    delay(2000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  IPAddress myAddr = WiFi.localIP();
  sprintf(buff, "%d.%d.%d.%d", myAddr[0], myAddr[1], myAddr[2], myAddr[3]);
  ip = String(buff);
  Serial.println(ip);
  Serial.write("\r\nStarte Webserver");
  server.begin();
  Serial.write("\r\nStarte Websocket");
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  Serial.write("\r\nErste Messung");
  int n=0;
  do {
    DS18B20.requestTemperatures();
    tempC = DS18B20.getTempCByIndex(0);
    delay(500);
    Serial.print(".");
    n++;
  }
  while (tempC==-127 && n<20);
  oldTempC = tempC;
  dtostrf(tempC, 2, 2, temperatureCString);
  Serial.println("\r\nTemperatur ist "+String(temperatureCString));
 
}

void loop() {
  webSocket.loop();
  WiFiClient clientS = server.available();

  // Kein neuer Client, dann Zeit zum Messen
  if (!clientS) {
    // Alle 15 Sek. eine Messung
    if (timer < millis()) {
      DS18B20.requestTemperatures();
      tempC = DS18B20.getTempCByIndex(0);
      Serial.println("Temperature is " + String(tempC));
      if (tempC!=-127) {
         timer = millis() + 15000;
        if (tempC != oldTempC) {
          oldTempC = tempC;
          dtostrf(tempC, 2, 2, temperatureCString);
          Serial.println("Temperature changed to " + String(temperatureCString));
          String s = getJson();
          webSocket.broadcastTXT(s);
        }
      }
      else {
        delay(500);
      }
    }

  }
  else {
    flasher = !flasher;
    // Interne LED bei jedem Request umschalten
    if (flasher) {
      digitalWrite(1, HIGH);
    }
    else {
      digitalWrite(1, LOW);
    }
    Serial.write("\r\nnew Client:");
    String request = clientS.readStringUntil('\r');
    Serial.println(">" + request + "<");
    String param = " ";
    if (request.indexOf("?") != -1) {
      param = request.substring(request.indexOf("?") + 1, request.indexOf(" HTTP/1.1"));
      if (param.indexOf("state=true") != -1) {
        digitalWrite(4, HIGH);
        state = true;
        String s = getJson();
        webSocket.broadcastTXT(s);
        clientS.print(s);
        clientS.flush();
        clientS.stop();
      }
      if (param.indexOf("state=false") != -1) {
        digitalWrite(4, LOW);
        state = false;
        String s = getJson();
        webSocket.broadcastTXT(s);
        clientS.print(s);
        clientS.flush();
        clientS.stop();
      }
    }
    else {
      Serial.println("\r\nparam >" + param + "<");
  
      String s;
      s = header_ok;
      s += contentTypeText;
      s += htmlhead;
      s += "<div class=\"container\" style=\"text-align: center;\"><div class=\"jumbotron\"><h1>ESP32</h1>";
  
      s += "<h3 id=\"idTemp\">Temperatur: " + String(temperatureCString) + " °C</h3></div></div><hr/>";
      s += "<div class=\"row\"><div class=\"col-md-6 col-md-offset-3\">";
      if (state) {
        s += "<button  id=\"on\" type=\"button\" class=\"btn btn-warning btn-lg btn-block\">OFF</button>";
      }
      else {
        s += "<button  id=\"on\" type=\"button\" class=\"btn btn-success btn-lg btn-block\">ON</button>";
      }
      s += "</div></div>";
      s += "<script>\r\n";
      s += "var state=" + String(state) + "\r\n";
      s += "console.log(\"state=\"+state);";
      s += " webSocket = new WebSocket(\"ws://" + ip + ":8080\");\r\n";
      s += " webSocket.onerror = function (event) {\r\n";
      s += "   console.log(\"Error, failed to Connect to Websocket\");\r\n";
      s += " }\r\n";
      s += " webSocket.onmessage = function (event) {\r\n";
      s += "    var obj = JSON.parse(event.data);";
      s += "    console.log(\"Websocket receive data:\" + JSON.stringify(obj));\r\n";
      s += "    $(\"#idTemp\").text(\"Temperature: \"+obj.temperature+\" °C\");\r\n";
      s += " state=obj.state;\r\n";
      s += " if (obj.state==1) {$(\"#on\").text(\"Off\");$(\"#on\").removeClass(\"btn-success\");$(\"#on\").addClass(\"btn-warning\");}\r\n";
      s += " else {$(\"#on\").text(\"On\");$(\"#on\").removeClass(\"btn-warning\");$(\"#on\").addClass(\"btn-success\");}\r\n";
      s += " };\r\n";
      s += " $(\"#on\").click(function () {\r\n";
      s += "   $.get(\"?state=\"+!state, function(data, status){\r\n";
      s += "     console.log(\"Data: \" + data + \"Status: \" + status);\r\n";
      s += "   });\r\n";
      s += " });\r\n";
      s += "</script>\r\n";
      s += htmltail;
      clientS.print(s);
      clientS.flush();
      clientS.stop();
    }
  }
}
