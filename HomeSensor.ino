/*
*	Не содержит ОТА функций
*
*	CSS811
*    VDD - подключается к выводу 3,3V.
*    GND - подключается к выводу GND.
*    SCL - подключается к линии тактирования SCL шины I2C
*    SDA - подключается к линии данных SDA шины I2C
*    WAK - подключается параллельно выводу GND
*    INT - не используется.
*    RST - не используется.
*    ADD - не используется.
*
*
*/
#include <SPI.h>
#include <Wire.h>

#include "Adafruit_CCS811.h"
// По умолчанию адрес датчика в библиотеке 0x5A

#include <ESP8266WiFi.h>

const char* ssid = "your-ssid";
const char* password = "your-password";

WiFiServer server(80);

Adafruit_CCS811 ccs;

void setup() {  
	Serial.begin(115200);
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


	if(!ccs.begin()){
		Serial.println("Failed to start sensor CCS811! Please check wiring.");
		while(1);
	}
  
  // по умолчанию мы будем генерировать высокое напряжение от линии 3.3v внутри! (Аккуратно!)
  
  //calibrate temperature sensor
	while(!ccs.available());
	float temp = ccs.calculateTemperature();
	ccs.setTempOffset(temp - 25.0);
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
	float output = 0;

	if(ccs.available()){
		float temp = ccs.calculateTemperature();
		if(!ccs.readData()){
			Serial.print("eCO2: ");
			if (req.indexOf("/eCO2") != -1)
				float eCO2 = ccs.geteCO2();
				output = eCO2;
				Serial.print(eCO2);
      
			else if (req.indexOf("/TVOC") != -1)
				Serial.print(" ppm, TVOC: ");      
				float TVOC = ccs.getTVOC();
				output = TVOC;
				Serial.print(TVOC);
		
			else if (req.indexOf("/temperature") != -1)
				Serial.print(" ppb   Temp:");
				output = temp;
				Serial.println(temp);
			else {
				Serial.println("invalid request");
				client.stop();
				return;
			}
		}
		else{
			Serial.println("ERROR!");
			while(1);
		}
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
