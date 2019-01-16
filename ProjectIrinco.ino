/*
	Project temperature sensing

	Describe what it does in layman's terms.  Refer to the components
	attached to the various pins.

	The circuit:
	* list the components attached to each input
	* list the components attached to each output

	Created 01/01/19
	By cdtekk
	Modified 01/16/19
	By cdtekk

*/

#define countof(a) (sizeof(a) / sizeof(a[0]))

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

File sensorDataFile;

RtcDS3231<TwoWire> Rtc(Wire);

int seconds = 0;
bool firstInit = true;

void setup() {
	softSerial.begin(74880);
	Serial.begin(74880);

	lcd.begin();
	lcd.backlight();

	lcdDisplay(0, 0, F("Initializing"), 0);
	lcdDisplay(0, 1, F("modules."), 1000);

	dht0.begin();
	dht1.begin();

	while (!SD.begin(4));

	Rtc.Begin();

	RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);

	if (!Rtc.IsDateTimeValid()) {
		Rtc.SetDateTime(compiled);
	}

	if (!Rtc.GetIsRunning()) {
		Rtc.SetIsRunning(true);
	}

	Rtc.Enable32kHzPin(false);
	Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone);

	lcd.clear();
	lcdDisplay(0, 1, F("Waiting ESP."), 0);

	while (1) {
		if (softSerial.available()) {
			String t = softSerial.readString();
			if (t.startsWith(F("IP"))) {
				lcdDisplay(0, 1, t.substring(4, 18), 2000);
				break;
			}
		}
	}
}

void loop() {
	if (seconds == 300000 || firstInit) {
		firstInit = false;
		seconds = 0;

		float h0 = dht0.readHumidity();
		float t0 = dht0.readTemperature();
		float h1 = dht1.readHumidity();
		float t1 = dht1.readTemperature();

		/*lcd.setCursor(0, 0);
		lcd.print(F("T1"));
		lcd.print("T1:" + String(t0, '\001') + " T2:" + String(t1, '\001'));
		lcd.setCursor(0, 1);
		lcd.print("H1:" + String(h0, '\001') + " H2:" + String(h1, '\001'));*/

		lcd.clear();
		lcd.setCursor(0, 0);
		lcd.print(F("TEMP:"));
		lcd.setCursor(6, 0);
		lcd.print((t0 + t1) / 2);

		lcd.setCursor(0, 1);
		lcd.print(F("HUM:"));
		lcd.setCursor(6, 1);
		lcd.print((h0 + h1) / 2);

		serialize(
			F("reading"),
			F("SensorData"),
			String(t0, '\001') + "," + String(t1, '\001') + "," + String(h0, '\001') + "," + String(h1, '\001')
		);

		sensorDataFile = SD.open(F("SensorData.txt"), FILE_WRITE);
		if (sensorDataFile) {
			sensorDataFile.print(formatTime(Rtc.GetDateTime()));
			sensorDataFile.println(String(t0, '\001') + "," + String(t1, '\001') + "," + String(h0, '\001') + "," + String(h1, '\001'));

			sensorDataFile.close();
		}
	}

	//Serial.println(formatTime(Rtc.GetDateTime()));

	delay(1000);
	seconds++;
}

String sdRead() {
	sensorDataFile = SD.open(F("SensorData.txt"));
	String data;
	if (sensorDataFile) {
		while (sensorDataFile.available()) {
			// Read SensorData.txt content
			data += sensorDataFile.readString();
		}

		sensorDataFile.close();
	}

	return data;
}

String formatTime(const RtcDateTime& dt) {
	char datestring[20];

	snprintf_P(datestring,
		countof(datestring),
		PSTR("%02u/%02u/%04u %02u:%02u"),
		dt.Month(),
		dt.Day(),
		dt.Year(),
		dt.Hour(),
		dt.Minute());
	return datestring;
}

void lcdDisplay(int col, int row, String caption, int _delay) {
	lcd.setCursor(col, row);
	lcd.print(caption);		//Use print function instead of println 
	delay(_delay);
}

void serialize(String contentType, String key, String value) {
	//Choose carefully the size. Might not work properly
	StaticJsonBuffer<60> jsonBuffer;
	JsonObject &root = jsonBuffer.createObject();
	root[key] = value;

	String document = F("reading\n");
	root.prettyPrintTo(document);
	softSerial.println(document);
	//Serial.println(document);
}