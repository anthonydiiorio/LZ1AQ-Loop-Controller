#include <WiFiManager.h> // v2.0 or higher https://github.com/tzapu/WiFiManager
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>

/*
	Setup
*/
const char *wifi_hostname = "loopcontroller";

ESP8266WebServer server(80);

char activeMode[10];
int windowHeight = 90;

/*
	Auxiliary Relay #4 Options
*/

bool auxEnable = true; //Enable 4th Relay in Web UI, set true or false
bool auxNC = true; //Sets visual indicator for Normally Closed
const char *auxLabel = "Power Supply"; //Label for Auxiliary Relay


/*
	GPIO Pins to use
	https://randomnerdtutorials.com/esp8266-pinout-reference-gpios/
*/

#define Relay1 14 // D5 GPIO14 Relay 1
#define Relay2 12 // D6 GPIO12 Relay 2
#define Relay3 13 // D7 GPIO13 Relay 3
#define Relay4 5  // D1 GPIO16 Relay 4 (AUX)

/*
	Set Loop modes

	Relays are Active Low

	1 0 0 Loop A 	(LOW, HIGH, HIGH)
	0 1 0 Loop B 	(HIGH, LOW, HIGH)
	1 1 0 A + B 	(LOW, LOW, HIGH)
	0 0 1 Vertical 	(HIGH, HIGH, LOW)

	Relay1 is connected normally closed to save power, no need to energize a relay in the default state.
	Since Relay1 is connected to Normally Closed we need to flip Relay1 (LOW becomes HIGH).
	Relays are activated with active low.
*/

void loopA(){ // 1 0 0 Active Low
	digitalWrite(Relay1, HIGH); // NC Flipped
	digitalWrite(Relay2, HIGH);
	digitalWrite(Relay3, HIGH);
	strcpy(activeMode, "A");
	//Serial.println("Loop A");
}

void loopB(){ // 0 1 0 Active Low
	digitalWrite(Relay1, LOW); // NC Flipped
	digitalWrite(Relay2, LOW);
	digitalWrite(Relay3, HIGH);
	strcpy(activeMode, "B");
	//Serial.println("Loop B");
}

void crossed(){ // 1 1 0 Active Low
	digitalWrite(Relay1, HIGH); // NC Flipped
	digitalWrite(Relay2, LOW);
	digitalWrite(Relay3, HIGH);
	strcpy(activeMode, "Crossed");
	//Serial.println("Crossed Parallel");
}

void vertical(){ // 0 0 1 Active Low
	digitalWrite(Relay1, LOW); // NC Flipped
	digitalWrite(Relay2, HIGH);
	digitalWrite(Relay3, LOW);
	strcpy(activeMode, "Vertical");
	//Serial.println("Vertical");
}

void auxToggle(){
	//Toggle Aux Relay on/off
	//Defaults to off, see void setup();
	digitalWrite(Relay4, !digitalRead(Relay4));
}

//Generate button string for active and inactive modes
String activeButton(const char *checkMode, const char *path, const char *btnText){
  String output = "<a class=\"btn ";
	if (strcmp(activeMode, checkMode) == 0){
    	output += "btn-success\" ";
	} else{
		output += "btn btn-dark\" ";
	}
	output +=  "href=\"/";
	output += path;
	output += "\">";
	output += btnText; 
	output += "</a> ";

	return output;
}

//Generate button string for Aux button
String auxButton(const char *path, const char *btnText){
	String output = "<a class=\"btn ";
	if (!digitalRead(Relay4)){
		if (auxNC){ //Invert Colors if Normally Closed
			output += "btn-dark\" ";
		} else{
			output += "btn-success\" ";
		}
	} else{
		if (auxNC){
			output += "btn-success\" ";
		} else{
			output += "btn btn-dark\" ";
		}
	}
	output +=  "href=\"/";
	output += path;
	output += "\">";
	output += btnText; 
	output += "</a> ";

	return output;
}

void handleRoot() {

	String html = "<!DOCTYPE html> <html> <head> <title>LZ1AQ Controller</title> <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"> <link rel=\"icon\" type=\"image/png\" href=\"https://i.imgur.com/nanrKpL.png\"> <link href=\"https://cdn.jsdelivr.net/npm/bootstrap@5.1.3/dist/css/bootstrap.min.css\" rel=\"stylesheet\" integrity=\"sha384-1BmE4kWBq78iYhFldvKuhfTAU6auU8tT94WrHftjDbrCEXSU1oBoqyl2QvZ6jIW3\" crossorigin=\"anonymous\"> <style type=\"text/css\"> body{ background: #000; color: #fff; } #main{ padding-top: 10px; margin: 0 auto; width: 310px; } </style> </head> <body> <div id=\"main\"> <h3>Loop Controller <a href=\"#\" onClick=\"Tiny=window.open('#','Tiny','width=330,height=";
	html += windowHeight;
	html += "'); return false;\">&#128377;</a></h3>";

	//Mode buttons (Mode, Path, Button Text)
	html += activeButton("A", "a", "Loop A");
	html += activeButton("B", "b", "Loop B");
	html += activeButton("Crossed", "x", "A + B");
	html += activeButton("Vertical", "v", "Vertical");

	if (auxEnable){
		html += "<div style=\"margin-top: 5px\"></div>";
		html += auxButton("aux", auxLabel); //(Path, Button Text)
	}

	//Close page
	html +="</div> </body> </html>";
	
	//Serve page
	server.send(200, "text/html", html);
}

void handleLoopA() {
  loopA();
  server.sendHeader("Location","/",true);
  server.send(303, "text/plain", "");
}

void handleLoopB() {
  loopB();
  server.sendHeader("Location","/",true);
  server.send(303, "text/plain", "");
}

void handleCrossed() {
  crossed();
  server.sendHeader("Location","/",true);
  server.send(303, "text/plain", "");
}

void handleVertical() {
  vertical();
  server.sendHeader("Location","/",true);
  server.send(303, "text/plain", "");
}

void handleAux() {
  auxToggle();
  server.sendHeader("Location","/",true);
  server.send(303, "text/plain", "");
}

void setup() {
	WiFi.hostname(wifi_hostname);
	WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP

	// put your setup code here, to run once:
	Serial.begin(115200);
	pinMode(Relay1, OUTPUT);
	pinMode(Relay2, OUTPUT);
	pinMode(Relay3, OUTPUT);
	pinMode(Relay4, OUTPUT);

	// Initialize Relays
	digitalWrite(Relay1, HIGH);
	digitalWrite(Relay2, HIGH);
	digitalWrite(Relay3, HIGH);
	digitalWrite(Relay4, HIGH); //Default Relay 4 to OFF, change to LOW to default ON.
	strcpy(activeMode, "A");

	// Set popout window height if aux button is enabled
	if (auxEnable){
		windowHeight = 135;
	}

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
	
	// Paths
	server.on("/", handleRoot);
	server.on("/a", handleLoopA);
	server.on("/b", handleLoopB);
	server.on("/x", handleCrossed);
	server.on("/v", handleVertical);
	server.on("/aux", handleAux);
	
	server.begin();
	//Serial.printf("Web server started, open %s in a web browser\n", WiFi.localIP().toString().c_str());
	
	MDNS.addService("http", "tcp", 80);
}

void loop() {
	server.handleClient();
	MDNS.update();
}
