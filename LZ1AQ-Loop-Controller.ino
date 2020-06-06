#include <WiFiManager.h> // v2.0 or higher https://github.com/tzapu/WiFiManager
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>

ESP8266WebServer server(80);

String activeMode = "";

#define Relay1 D5 //Relay 1
#define Relay2 D6 //Relay 2
#define Relay3 D7 //Relay 3
#define RelayTest 

void setup() {
	char* wifi_hostname = "loopdev";

	WiFi.hostname(wifi_hostname);
	WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP

	// put your setup code here, to run once:
	Serial.begin(115200);

	//WiFiManager, Local intialization. Once its business is done, there is no need to keep it around
	WiFiManager wm;	

	//reset settings - wipe credentials for testing
	// wm.resetSettings();	

	// Automatically connect using saved credentials,
	// if connection fails, it starts an access point with the specified name ("AutoConnectAP"),
	// if empty will auto generate SSID, if password is blank it will be anonymous AP (wm.autoConnect())
	// then goes into a blocking loop awaiting configuration and will return success result	
	bool res;
	res = wm.autoConnect("LoopController","lz1aqconfig"); // password protected ap	
	if(!res) {
		Serial.println("Failed to connect");
		// ESP.restart();
	} 
	else {  
		Serial.println("Connected :)");
	}	
	
	if (!MDNS.begin(wifi_hostname)) {
		Serial.println("Error setting up MDNS responder!");
		while (1) {
			delay(1000);
		}
	} else {
		Serial.println("mDNS responder started");	
	}
	
	server.on("/", handleRoot); //Main page
	server.on("/LoopA", handleLoopA);
	server.on("/LoopB", handleLoopB);
	server.on("/LoopAB", handleLoopAB);
	server.on("/Vertical", handleVertical);
	
	server.begin();
	Serial.printf("Web server started, open %s in a web browser\n", WiFi.localIP().toString().c_str());
	
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
  // digitalWrite(Relay1, LOW); // Normally Open
  digitalWrite(Relay1, HIGH); // NC
  digitalWrite(Relay2, HIGH);
  digitalWrite(Relay3, HIGH);
  activeMode = "A";
  Serial.println("Loop A");
}

void loopB() { // 0 1 0 Active Low
  // digitalWrite(Relay1, HIGH); // Normally Open
  digitalWrite(Relay1, LOW); // NC
  digitalWrite(Relay2, LOW);
  digitalWrite(Relay3, HIGH);
  activeMode = "B";
  Serial.println("Loop B");
}

void loopAB() { // 1 1 0 Active Low
  // digitalWrite(Relay1, LOW); // Normally Open
  digitalWrite(Relay1, HIGH); // NC
  digitalWrite(Relay2, LOW);
  digitalWrite(Relay3, HIGH);
  activeMode = "AB";
  Serial.println("Loop AB");
}

void vertical() { // 0 0 1 Active Low
  // digitalWrite(Relay1, HIGH); // Normally Open
  digitalWrite(Relay1, LOW); // NC
  digitalWrite(Relay2, HIGH);
  digitalWrite(Relay3, LOW);
  activeMode = "V";
  Serial.println("Vertical");
}
