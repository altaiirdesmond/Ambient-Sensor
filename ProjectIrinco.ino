#include <ArduinoJson.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>
#include <RtcDS3231.h>
#include <SD.h>
#include <SPI.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <string.h>

SoftwareSerial softSerial(8, 9);

LiquidCrystal_I2C lcd(0x27, 16, 2);

DHT dht0(6, DHT22);
DHT dht1(7, DHT22);

File sdFile;

void setup() {
	softSerial.begin(74880);
	Serial.begin(115200);

	lcd.begin();
	lcd.backlight();

	lcdDisplay(0, 0, "Initializing", 0);
	lcdDisplay(0, 1, "modules.", 1000);

	dht0.begin();
	dht1.begin();

	//if (!SD.begin(10)) {
	//	lcdDisplay(0, 0, "SD Failed.", 0);
	//	lcdDisplay(0, 1, "Try reseating SDmod.", 0);
	//	// Serial.println("SD initialization failed!");
	//	while (1)
	//		;
	//}

	lcd.clear();
	lcdDisplay(0, 0, "Turn On/Rst ESP", 0);
	while (1) {
		if (softSerial.available()) {
			String t = softSerial.readString();
			lcdDisplay(0, 1, "acquiring IP...", 0);
			if (t.startsWith("IP")) {
				String ip = t.substring(4, 18);
				lcd.clear();
				lcdDisplay(0, 0, "Connect at", 0);
				lcdDisplay(0, 1, ip, 0);
				break;
			}
		}
	}
}

void loop() {
	// Reading temperature or humidity takes about 250 milliseconds!
	// Sensor readings may also be up to 2 seconds 'old' (its a very slow
	// sensor)
	delay(2000);
	float h0 = dht0.readHumidity();
	float t0 = dht0.readTemperature();
	float h1 = dht1.readHumidity();
	float t1 = dht1.readTemperature();

	lcd.setCursor(0, 0);
	lcd.print("T1:" + String(t0, '\001') + " T2:" + String(t1, '\001'));
	lcd.setCursor(0, 1);
	lcd.print("H1:" + String(h0, '\001') + " H2:" + String(h1, '\001'));

	serialize(
		"reading",
		"SensorData",
		String(t0, '\001') + "," + String(t1, '\001') + "," + String(h0, '\001') + "," + String(h1, '\001')
	);
}

void lcdDisplay(int col, int row, String caption, int _delay) {
	lcd.setCursor(col, row);
	lcd.print(caption);		//Use print function instead of println 
	delay(_delay);
}

void serialize(String contentType, String key, String value) {
	//Causes arduino to reset if the buffer is < 90s
	StaticJsonBuffer<90> jsonBuffer;
	JsonObject &root = jsonBuffer.createObject();
	root[key] = value;

	String document = "reading\n";
	root.prettyPrintTo(document);
	softSerial.println(document);
}
