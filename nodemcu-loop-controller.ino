#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>

// Wifi: SSID and password
const char* ssid = "SSID"; //Replace
const char* password = "PASSWORD"; //Replace

String activeMode = "";

ESP8266WebServer server(80);

#define RELAY1 D5 //Relay 1
#define RELAY2 D6 //Relay 2
#define RELAY3 D7 //Relay 3

void setup() {
  Serial.begin(115200);
  Serial.println();
  
  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);
  pinMode(RELAY3, OUTPUT);
  
  //Default Mode Loop A
  loopA();
  
  WiFi.hostname("loopcontroller");
  
  Serial.printf("Connecting to %s ", ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" connected");

  if (!MDNS.begin("loopcontroller")) {
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");
 
  server.on("/", handleRoot); //Main page
  server.on("/LoopA", handleLoopA);
  server.on("/LoopB", handleLoopB);
  server.on("/LoopAB", handleLoopAB);
  server.on("/Vertical", handleVertical);
  
  server.begin();
  Serial.printf("Web server started, open %s in a web browser\n", WiFi.localIP().toString().c_str());

  // Add service to MDNS-SD
  MDNS.addService("http", "tcp", 80);
}

void loop() {
  server.handleClient();
}

void handleRoot() {
  //Start page
  String html ="<!DOCTYPE html> <html> <head> <title>LZ1AQ Controller</title> <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"> <link rel=\"stylesheet\" href=\"https://stackpath.bootstrapcdn.com/bootstrap/4.3.1/css/bootstrap.min.css\" integrity=\"sha384-ggOyR0iXCbMQv3Xipma34MD+dH/1fQ784/j6cY/iJTQUOhcWr7x9JvoRxT2MZw1T\" crossorigin=\"anonymous\"> <style type=\"text/css\"> body{ background: #000; color: #fff; } #main{ padding-top: 10px; margin: 0 auto; width: 315px; } </style> </head> <body> <div id=\"main\"> <h3>Loop Controller <a href=\"#\" onClick=\"MyWindow=window.open('#','MyWindow','width=330,height=90'); return false;\">&#128377;</a></h3>";  

  //Mode buttons (Mode, URI, Button Text)
  html += activeButton("A", "LoopA", "Loop A");
  html += activeButton("B", "LoopB", "Loop B");
  html += activeButton("AB", "LoopAB", "A + B");
  html += activeButton("V", "Vertical", "Vertical");

  //Close page
  html +="</div> </body> </html>";

  //Serve page
  server.send(200, "text/html", html);
}

//Generate button string for active and inactive modes
String activeButton(String checkMode, String href, String btnText){
  String output = "<a class=\"btn ";
  if (activeMode == checkMode){
    output += "btn-success\" ";
  } else{
    output += "btn btn-dark\" ";
  }
  output +=  "href=\"/" + href + "\">" + btnText + "</a> ";
  return output;
}

void handleLoopA() {
  loopA();
  server.sendHeader("Location","/");
  server.send(303);
}

void handleLoopB() {
  loopB();
  server.sendHeader("Location","/");
  server.send(303);
}

void handleLoopAB() {
  loopAB();
  server.sendHeader("Location","/");
  server.send(303);
}

void handleVertical() {
  vertical();
  server.sendHeader("Location","/");
  server.send(303);
}

void loopA() { // 1 0 0 Active Low
  // digitalWrite(RELAY1, LOW); // Normally Open
  digitalWrite(RELAY1, HIGH); // NC
  digitalWrite(RELAY2, HIGH);
  digitalWrite(RELAY3, HIGH);
  activeMode = "A";
  Serial.println("Loop A");
}

void loopB() { // 0 1 0 Active Low
  // digitalWrite(RELAY1, HIGH); // Normally Open
  digitalWrite(RELAY1, LOW); // NC
  digitalWrite(RELAY2, LOW);
  digitalWrite(RELAY3, HIGH);
  activeMode = "B";
  Serial.println("Loop B");
}

void loopAB() { // 1 1 0 Active Low
  // digitalWrite(RELAY1, LOW); // Normally Open
  digitalWrite(RELAY1, HIGH); // NC
  digitalWrite(RELAY2, LOW);
  digitalWrite(RELAY3, HIGH);
  activeMode = "AB";
  Serial.println("Loop AB");
}

void vertical() { // 0 0 1 Active Low
  // digitalWrite(RELAY1, HIGH); // Normally Open
  digitalWrite(RELAY1, LOW); // NC
  digitalWrite(RELAY2, HIGH);
  digitalWrite(RELAY3, LOW);
  activeMode = "V";
  Serial.println("Vertical");
}
