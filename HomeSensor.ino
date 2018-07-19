/*
*	Не содержит ОТА функций
*
*	BME280 by Tyler Glenn
*    VDD - подключается к выводу 3,3V.
*    GND - подключается к выводу GND.
*    SCL - подключается к линии тактирования SCL шины I2C
*    SDA - подключается к линии данных SDA шины I2C
*    WAK - подключается параллельно выводу GND
*    INT - не используется.
*    RST - не используется.
*    ADD - не используется.
*	SHT21 (SHT1x and SHT7x)
*    адрес I2C фиксированный 0x80
0xE3 — Измерить температуру. При этом на время измерения линия SCL будет прижата к земле.
0xE5 — Измерить влажность. Линия SCL так-же прижимается к земле на время измерения.
0xF3 — Измерить температуру. На этот раз датчик не прижимает линию SCL на время замера.
0xF5 — Измерить влажность. Линия SCL не прижимается.
0xE6 — Записать данные в пользовательский регистр.
0xE7 — Прочитать данные из пользовательского регистра.
0xFE — Перезагрузить датчик. 
*
*/
#include <SPI.h>
#include <Wire.h>

#include <Adafruit_CCS811.h>
// По умолчанию адрес датчика в библиотеке 0x5A
#include <Sodaq_SHT2x.h>


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
