#include <WiFiManager.h> // v2.0 or higher https://github.com/tzapu/WiFiManager
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>

/*
	Setup
*/
const char *wifi_hostname = "loopcontroller";

ESP8266WebServer server(80);

char activeMode[10];

#define Relay1 14 //D5 GPIO 14 Relay 1
#define Relay2 12 //D6 GPIO 12 Relay 2
#define Relay3 13 //D7 GPIO 13 Relay 3

/*
	Set Loop modes

	Relays are Active Low

	1 0 0 Loop A 	(LOW, HIGH, HIGH)
	0 1 0 Loop B 	(HIGH, LOW, HIGH)
	1 1 0 A + B 	(LOW, LOW, HIGH)
	0 0 1 Vertical 	(HIGH, HIGH, LOW)

	Since Relay1 is set to NC we need to flip Relay1.
*/

void loopA() { // 1 0 0 Active Low
	// digitalWrite(Relay1, LOW); // Normally Open
	digitalWrite(Relay1, HIGH); // NC
	digitalWrite(Relay2, HIGH);
	digitalWrite(Relay3, HIGH);
	strcpy(activeMode, "A");
	Serial.println("Loop A");
}

void loopB() { // 0 1 0 Active Low
	// digitalWrite(Relay1, HIGH); // Normally Open
	digitalWrite(Relay1, LOW); // NC
	digitalWrite(Relay2, LOW);
	digitalWrite(Relay3, HIGH);
	strcpy(activeMode, "B");
	Serial.println("Loop B");
}

void crossed() { // 1 1 0 Active Low
	// digitalWrite(Relay1, LOW); // Normally Open
	digitalWrite(Relay1, HIGH); // NC
	digitalWrite(Relay2, LOW);
	digitalWrite(Relay3, HIGH);
	strcpy(activeMode, "Crossed");
	Serial.println("Crossed Parallel");
}

void vertical() { // 0 0 1 Active Low
	// digitalWrite(Relay1, HIGH); // Normally Open
	digitalWrite(Relay1, LOW); // NC
	digitalWrite(Relay2, HIGH);
	digitalWrite(Relay3, LOW);
	strcpy(activeMode, "Vertical");
	Serial.println("Vertical");
}

//Generate button string for active and inactive modes
String activeButton(const char *checkMode, String href, String btnText){
  String output = "<a class=\"btn ";
	if (strcmp(activeMode, checkMode) == 0){
    	output += "btn-success\" ";
	} else{
		output += "btn btn-dark\" ";
	}
	output +=  "href=\"/" + href + "\">" + btnText + "</a> ";
	return output;
}

void handleRoot() {
  //Start page
  String html ="<!DOCTYPE html> <html> <head> <title>LZ1AQ Controller</title> <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"> <link rel=\"stylesheet\" href=\"https://stackpath.bootstrapcdn.com/bootstrap/4.5.0/css/bootstrap.min.css\" integrity=\"sha384-9aIt2nRpC12Uk9gS9baDl411NQApFmC26EwAOH8WgZl5MYYxFfc+NcPb1dKGj7Sk\" crossorigin=\"anonymous\"> <style type=\"text/css\"> body{ background: #000; color: #fff; } #main{ padding-top: 10px; margin: 0 auto; width: 315px; } </style> </head> <body> <div id=\"main\"> <h3>Loop Controller <a href=\"#\" onClick=\"MyWindow=window.open('#','MyWindow','width=330,height=90'); return false;\">&#128377;</a></h3>";  

  //Mode buttons (Mode, URI, Button Text)
  html += activeButton("A", "LoopA", "Loop A");
  html += activeButton("B", "LoopB", "Loop B");
  html += activeButton("Crossed", "Crossed", "A + B");
  html += activeButton("Vertical", "Vertical", "Vertical");

  //Close page
  html +="</div> </body> </html>";

  //Serve page
  server.send(200, "text/html", html);
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

void handlecrossed() {
  crossed();
  server.sendHeader("Location","/");
  server.send(303);
}

void handleVertical() {
  vertical();
  server.sendHeader("Location","/");
  server.send(303);
}

void setup() {
	WiFi.hostname(wifi_hostname);
	WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP

	// put your setup code here, to run once:
	Serial.begin(115200);
	pinMode(Relay1, OUTPUT);
	pinMode(Relay2, OUTPUT);
	pinMode(Relay3, OUTPUT);

	// Initialize Relays
	digitalWrite(Relay1, HIGH);
	digitalWrite(Relay2, HIGH);
	digitalWrite(Relay3, HIGH);
	strcpy(activeMode, "A");

	//WiFiManager, Local intialization. Once its business is done, there is no need to keep it around
	WiFiManager wifiManager;	

	//reset settings - wipe credentials for testing
	// wifiManager.resetSettings();	

	// Automatically connect using saved credentials,
	// if connection fails, it starts an access point with the specified name ("LoopController"),
	// if empty will auto generate SSID, if password is blank it will be anonymous AP (wifiManager.autoConnect())
	// then goes into a blocking loop awaiting configuration and will return success result	
	bool res;
	res = wifiManager.autoConnect("LoopController","loopconfig"); // password protected ap	
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
	server.on("/Crossed", handlecrossed);
	server.on("/Vertical", handleVertical);
	
	server.begin();
	Serial.printf("Web server started, open %s in a web browser\n", WiFi.localIP().toString().c_str());
	
	MDNS.addService("http", "tcp", 80);
}

void loop() {
	server.handleClient();
  MDNS.update();
}
