#include <ArduinoJson.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>
#include <RtcDS3231.h>
#include <SD.h>
#include <SPI.h>
#include <SoftwareSerial.h>
#include <Wire.h>

SoftwareSerial softSerial(8, 9);

LiquidCrystal_I2C lcd(0x27, 16, 2);

DHT dht0(6, DHT22);
DHT dht1(7, DHT22);

void setup() {
  softSerial.begin(74880);
  Serial.begin(74880);

  lcd.begin();
  lcd.backlight();

  lcdDisplay(0, 0, "initializing", 0);
  lcdDisplay(0, 1, "modules", 0);

  dht0.begin();
  dht1.begin();

  lcdDisplay(0, 0, "Waiting IP.", 0);
  while (1) {
    if (softSerial.available()) {
      String t = softSerial.readString();
      lcdDisplay(0, 1, "acquiring...", 0);
      if (t.startsWith("IP")) {
        String ip = t.substring(4, 18);
        lcd.clear();
        lcdDisplay(0, 0, "Connect at", 0);
        lcdDisplay(0, 1, ip, 3000);
        break;
      }
    }
  }
}

void loop() {
  delay(2000);
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow
  // sensor)
  float h0 = dht0.readHumidity();
  float t0 = dht0.readTemperature();
  float h1 = dht1.readHumidity();
  float t1 = dht1.readTemperature();

  lcdDisplay(0, 0, "T1:" + String(t0) + "T2:" + String(t1), 0);
  lcdDisplay(0, 1, "H1:" + String(h0) + "H2:" + String(h1), 0);
}

void lcdDisplay(int col, int row, String caption, int _delay) {
  lcd.setCursor(col, row);
  lcd.println(caption);
  delay(_delay);
}
