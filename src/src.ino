/*
  Project temperature sensing

  Describe what it does in layman's terms.  Refer to the components
  attached to the various pins.

  The circuit:
   list the components attached to each input
   list the components attached to each output

  Created 01/01/19
  By ...
  Modified 01/16/19
  By ...

*/

#define countof(a) (sizeof(a) / sizeof(a[0]))

#include <ArduinoJson.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>
#include <RtcDS3231.h>
#include <SPI.h>
#include <Wire.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

DHT dht0(6, DHT22);
DHT dht1(7, DHT22);

RtcDS3231<TwoWire> Rtc(Wire);

const int relay0 = 8;
const int relay1 = 9;

unsigned long lastDisplayUpdate = 0;
unsigned long lastFanUpdate = 0;
bool firstInit = true;

void setup() {
  pinMode(relay0, OUTPUT);
  pinMode(relay1, OUTPUT);

  // softSerial.begin(74880);
  Serial.begin(74880);
  Serial1.begin(74880);

  lcd.begin();
  lcd.backlight();

  lcdDisplay(0, 0, F("Initializing"), 0);
  lcdDisplay(0, 1, F("modules."), 1000);

  dht0.begin();
  dht1.begin();

  Rtc.Begin();

  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);

  if (!Rtc.IsDateTimeValid()) {
    Rtc.SetDateTime(compiled);
  }

  if (!Rtc.GetIsRunning()) {
    Rtc.SetIsRunning(true);
  }

  RtcDateTime now = Rtc.GetDateTime();
  if (now < compiled) {
    Rtc.SetDateTime(compiled);
  }

  Rtc.Enable32kHzPin(false);
  Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone);

  lcd.clear();
  lcdDisplay(0, 0, F("Waiting ESP..."), 0);
  while (1) {
    if (Serial1.available()) {
      lcdDisplay(0, 1, F("acquiring IP"), 0);
      String t = Serial1.readString();
      if (t.startsWith(F("IP"))) {
        lcd.clear();
        lcdDisplay(0, 0, F("Connect to"), 0);
        lcdDisplay(0, 1, t.substring(4, 18), 2000);
        break;
      }
    }
  }
}

void loop() {
  unsigned long currentMillis = millis(); // Gets track of time
  if ((unsigned long)(currentMillis - lastDisplayUpdate) >= 300000 || firstInit) {
    firstInit = false;
    lastDisplayUpdate = currentMillis; // Set new time at which to run this block

    float h0 = dht0.readHumidity();
    float t0 = dht0.readTemperature();
    float h1 = dht1.readHumidity();
    float t1 = dht1.readTemperature();

    lcd.clear();
    lcdDisplay(0, 0, F("Logged."), 500);

    // Display sensor values to LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("TEMPERATURE:"));
    lcd.setCursor(12, 0);
    int averageT = (t0 + t1) / 2;
    lcd.print(averageT);

    lcd.setCursor(0, 1);
    lcd.print(F("HUMIDITY:"));
    lcd.setCursor(12, 1);
    int averageH = (h0 + h1) / 2;
    lcd.print(averageH);

    // POST sensor values to server
    DynamicJsonBuffer jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();
    root["temperature"] = averageT;
    root["humidity"] = averageH;
    root["time"] = formatTime(Rtc.GetDateTime());
    
    String document = F("reading");
    root.printTo(document);
    Serial1.println(document);
    Serial.println(document);
  }

  if ((unsigned long)(currentMillis - lastFanUpdate) >= 500) {
    lastFanUpdate = currentMillis;

    if (Serial1.available()) {
      String x = Serial1.readString();
      if (x.startsWith(F("FanControl"))) {
        pinMode(relay0, x.substring(11, 12).toInt());
        pinMode(relay1, x.substring(13, 14).toInt());

        String data = x.substring(11, 12) + F(",") + x.substring(13, 14);
        Serial.println(data);
      }
    }
  }
}

String formatTime(const RtcDateTime& dt) {
  char datestring[20];

  snprintf_P(datestring,
             countof(datestring),
             PSTR("%02u/%02u/%04u,%02u:%02u"),
             dt.Month(),
             dt.Day(),
             dt.Year(),
             dt.Hour(),
             dt.Minute());
  return datestring;
}

void lcdDisplay(int col, int row, String caption, int _delay) {
  lcd.setCursor(col, row);
  lcd.print(caption);   //Use print function instead of println
  delay(_delay);
}
