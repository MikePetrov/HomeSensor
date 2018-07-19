/*
*	Не содержит ОТА функций
*
*	CCS811
*    VDD - подключается к выводу 3,3V.
*    GND - подключается к выводу GND.
*    SCL - подключается к линии тактирования SCL шины I2C
*    SDA - подключается к линии данных SDA шины I2C
*    WAK - подключается параллельно выводу GND
*    INT - не используется.
*    RST - не используется.
*    ADD - не используется.
*	
	BME280 by Tyler Glenn library
Connecting the BME280 Sensor:
Sensor              ->  Board
-----------------------------
Vin (Voltage In)    ->  3.3V
Gnd (Ground)        ->  Gnd
SDA (Serial Data)   ->  D2 on ESP8266
SCK (Serial Clock)  ->  D1 on ESP8266
*
*/

#include <Wire.h>

#include <EnvironmentCalculations.h>
#include <BME280I2C.h>

#define SERIAL_BAUD 115200

#include <ESP8266WiFi.h>

const char* ssid = "your-ssid";
const char* password = "your-password";

WiFiServer server(80);

BME280I2C bme;    // Default : forced mode, standby time = 1000 ms
                  // Oversampling = pressure ×1, temperature ×1, humidity ×1, filter off,


void setup() {  
	Serial.begin(SERIAL_BAUD);
	while(!Serial) {} // Ждемс...! (на период отладки)
	
	Wire.begin();
	
	Serial.println();
  
	Serial.print("Connecting to ");
	Serial.println(ssid);
	
	WiFi.begin(ssid, password);
	
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}
	Serial.println("");
	Serial.println("WiFi connected");
	
	server.begin();
	Serial.println("Server started");

	Serial.println(WiFi.localIP());


	if(!bme.begin()){
		Serial.println("Failed to start or not find BME280 sensor! Please check wiring.");
		while(1);
	}
  
}

void loop() {
	WiFiClient client = server.available();
	if (!client) {
		return;
	}

	while(!client.available()){
		delay(1);
	}
	
	String req = client.readStringUntil('\r');
	client.flush();
	
	float data = 0;
	float output = 0;

	if (req.indexOf("/humidity") != -1)
		float output = SHT2x.GetHumidity();
	else if (req.indexOf("/temperature1") != -1)
		float output = SHT2x.GetTemperature();
	else if (req.indexOf("/dewpoint") != -1)
		float output = SHT2x.GetDewPoint();
	
	else if (req.indexOf("/eCO2") != -1)
		if(ccs.available()){
			float data = ccs.geteCO2();
			if(!ccs.readData()){
				Serial.print("eCO2: ");
				float output = data;
				Serial.print(data);
			}
			else {
				Serial.println("invalid request to CCS811");
				return;
			}
		}
	else if (req.indexOf("/TVOC") != -1)
		if(ccs.available()){
			float output = ccs.getTVOC();
			if(!ccs.readData()){
				Serial.print("TVOC: ");
				float output = data;
				Serial.print(data);
			}
			else {
				Serial.println("invalid request to CCS811");
				return;
			}
		}
	else if (req.indexOf("/temperature2") != -1)
		if(ccs.available()){
			float output = ccs.calculateTemperature();
			if(!ccs.readData()){
				Serial.print("Temp: ");
				float output = data;
				Serial.print(data);
			} 
			else {
				Serial.println("invalid request to CCS811");
				return;
			}
		}
	else{
		Serial.println("ERROR! NO DATA !!!");
		while(1);
		}
	
delay(500);

	client.flush();
	String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\n";
	s += output;
	s += "</html>\n";
	
	client.print(s);
	delay(1);
	Serial.println("Client disonnected");

}
