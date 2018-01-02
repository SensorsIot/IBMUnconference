/* DHT Pro Shield - Thingspeak

   Example testing sketch for various DHT humidity/temperature sensors
   Written by ladyada, public domain

   Depends on Adafruit DHT Arduino library
   https://github.com/adafruit/DHT-sensor-library
*/

#define APPNAME "TempSensor"
#define VERSION "V1.1.0"
#define COMPDATE __DATE__ __TIME__
#define MODEBUTTON D3

#include <SSD1306.h>
#include <IOTAppStory.h>
#include <DHT.h>
#include <ESP8266WiFi.h>
//#include <PubSubClient.h>
#include <credentials.h>

#define DHTPIN D4
// Uncomment whatever type you're using!
#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
//#define DHTTYPE DHT22   // DHT 22  (AM2302)

#define PIN_RESET 255  //
#define DC_JUMPER 0  // I2C Addres: 0 - 0x3C, 1 - 0x3D

float h, t, f, hic, hif;

DHT dht(DHTPIN, DHTTYPE);
IOTAppStory IAS(APPNAME, VERSION, COMPDATE, MODEBUTTON);
SSD1306  display(0x3c, D2, D1);

int tempEntry;
unsigned long iotEntry = millis();

void displayTemp(float temp) {
  display.clear();
  display.setFont(ArialMT_Plain_24);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(32, 15, String(temp));
  display.setFont(ArialMT_Plain_16);
  display.drawString(32, 40, "Celsius");
  display.display();
}

void displayConfig() {
  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(32, 15, "Config");
  display.setFont(ArialMT_Plain_16);
  display.drawString(32, 40, "Mode");
  display.display();
}

void displayUpdatate() {
  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(32, 15, "Update");
  display.setFont(ArialMT_Plain_16);
  display.drawString(32, 40, "Sketch");
  display.display();
}

void printLog() {
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(" *C ");
  Serial.print(f);
  Serial.print(" *F\t");
  Serial.print("Heat index: ");
  Serial.print(hic);
  Serial.print(" *C ");
  Serial.print(hif);
  Serial.println(" *F");
  tempEntry = millis();
}

void setup() {
  IAS.serialdebug(true);                  // 1st parameter: true or false for serial debugging. Default: false
  String boardName = APPNAME"_" + WiFi.macAddress();
  IAS.preSetBoardname(boardName);
  IAS.preSetAutoUpdate(false);                     // automaticUpdate (true, false)
  IAS.preSetAutoConfig(false);                      // automaticConfig (true, false)
  IAS.preSetWifi(mySSID, myPASSWORD);            // preset Wifi

  IAS.begin(true,'P');

  IAS.onModeButtonConfigMode([]() {
    displayConfig();
  });

  IAS.onModeButtonFirmwareUpdate([]() {
    displayUpdatate();
  });

  //-------- Your Setup starts from here ---------------

  display.init();
  display.flipScreenVertically();
  dht.begin();
  // Before you can start using the OLED, call begin() to init
  // all of the pins and configure the OLED.

  Serial.println("Setup done");
}

void loop() {
  IAS.buttonLoop();
  delay(1);

  if (millis() > iotEntry + 30000) {             // only for development. Please change it to at least 2 hours in production
    //  IAS.callHome();
    iotEntry = millis();
  }

  if (millis() > tempEntry + 5000) { // Wait a few seconds between measurements.

    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    do {
      h = dht.readHumidity();
    } while (isnan(h));
    // Read temperature as Celsius (the default)
    do {
      t = dht.readTemperature();
    } while (isnan(t));
    // Read temperature as Fahrenheit (isFahrenheit = true)
    do {
      f = dht.readTemperature(true);
    } while (isnan(f));

    // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t) || isnan(f)) Serial.println("Failed to read from DHT sensor!");

    // Compute heat index in Fahrenheit (the default)
    hif = dht.computeHeatIndex(f, h);
    // Compute heat index in Celsius (isFahreheit = false)
    hic = dht.computeHeatIndex(t, h, false);

    displayTemp(t);
    printLog();
  }
}
