/* DHT Pro Shield - Thingspeak

   Example testing sketch for various DHT humidity/temperature sensors
   Written by ladyada, public domain

   Depends on Adafruit DHT Arduino library
   https://github.com/adafruit/DHT-sensor-library
*/

#define APPNAME "WemosTempSensorThinspeak"
#define VERSION "V1.2.0"
#define COMPDATE __DATE__ __TIME__
#define MODEBUTTON D3

#include <SSD1306.h>
#include <IOTAppStory.h>
#include <DHT.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <credentials.h>

#define DHTPIN D4
// Uncomment whatever type you're using!
#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
//#define DHTTYPE DHT22   // DHT 22  (AM2302)

#define PIN_RESET 255  //
#define DC_JUMPER 0  // I2C Addres: 0 - 0x3C, 1 - 0x3D

float h, t, f, hic, hif;

#ifndef CREDENTIALS
#define mySSID "*****"
#define myPASSWORD "*****"
#endif

#define WRITEAPIKEY "0IMIFX8LEKF08YTX"

#define OUTTOPIC "channels/<channelID/publish/"WRITEAPIKEY
const char* mqtt_server = "mqtt.thingspeak.com";
char msg[50] = "field1=22.5&field2=65.7&status=MQTTPUBLISH";

WiFiClient espClient;
PubSubClient thingspeak(espClient);

DHT dht(DHTPIN, DHTTYPE);
IOTAppStory IAS(APPNAME, VERSION, COMPDATE, MODEBUTTON);
SSD1306  display(0x3c, D2, D1);

int tempEntry;
unsigned long iotEntry = millis();

void displayError() {
  display.clear();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(32, 15, F("Error"));
  display.drawString(32, 30, F("Reding"));
  display.drawString(32, 45, F("Sensor"));
  display.display();
}

void displayTemp(float temp) {
  display.clear();
  display.setFont(ArialMT_Plain_24);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(32, 15, String(temp));
  display.setFont(ArialMT_Plain_16);
  display.drawString(32, 40, "Celsius");
  display.display();
}

void displayStartup() {
  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(32, 15, F("Press"));
  display.drawString(32, 30, F("Reset"));
  display.drawString(32, 45, F("Button"));
  display.display();
}

void displayUpdate() {
  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(32, 15, F("Update"));
  display.drawString(52, 30, F("of"));
  display.drawString(42, 45, F("App"));
  display.display();
}

void displayConfigMode() {
  display.clear();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(32, 15, F("Connect to"));
  display.setFont(ArialMT_Plain_16);
  display.drawString(40, 30, F("Wi-Fi"));
  display.setFont(ArialMT_Plain_10);
  display.drawString(32, 50, "x:x:" + WiFi.macAddress().substring(9, 99));
  display.display();
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!thingspeak.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (thingspeak.connect("ESP8266Client")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      thingspeak.publish(OUTTOPIC, "hello world");
      // ... and resubscribe
      thingspeak.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(thingspeak.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void publishToThingspeak() {
  if (!thingspeak.connected()) {
    reconnect();
  }
  thingspeak.loop();
  snprintf (msg, 75, "field1=%f&field2=%f&status=MQTTPUBLISH", t, h);
  Serial.print("Publish message: ");
  Serial.println(msg);
  delay(1);
  thingspeak.publish(OUTTOPIC, msg);
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
}

void setup() {
  IAS.serialdebug(true);                  // 1st parameter: true or false for serial debugging. Default: false
  display.init();
  display.flipScreenVertically();
  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(48, 35, F("Wait"));
  display.display();

  String boardName = APPNAME"_" + WiFi.macAddress();
  IAS.preSetBoardname(boardName);
  IAS.preSetAutoUpdate(false);
  IAS.preSetAutoConfig(true);
  IAS.preSetWifi(mySSID, myPASSWORD);

  /*
     IAS.onFirstBoot([]() {
       Serial.println(F(" Hardware reset necessary after Serial upload. Reset to continu!"));
       Serial.println(F("*-------------------------------------------------------------------------*"));
       displayStartup();
       while (1) yield();
     });
  */

  IAS.onConfigMode([]() {
    displayConfigMode();
  });

  IAS.onFirmwareUpdate([]() {
    displayUpdate();
  });


  IAS.onModeButtonShortPress([]() {
    Serial.println(F(" If mode button is released, I will enter in firmware update mode."));
    Serial.println(F("*-------------------------------------------------------------------------*"));
  });

  IAS.onModeButtonLongPress([]() {
    Serial.println(F(" If mode button is released, I will enter in configuration mode."));
    Serial.println(F("*-------------------------------------------------------------------------*"));
  });

  IAS.onModeButtonVeryLongPress([]() {
    Serial.println(F(" If mode button is released, I won't do anything unless you program me to."));
    Serial.println(F("*-------------------------------------------------------------------------*"));
    /* TIP! You can use this callback to put your app on it's own configuration mode */
  });

  IAS.begin(true, 'P');

  //-------- Your Setup starts from here ---------------
  dht.begin();
  // Before you can start using the OLED, call begin() to init
  // all of the pins and configure the OLED.

  display.init();
  display.flipScreenVertically();

  Serial.println("defining MQTT");
  thingspeak.setServer(mqtt_server, 1883);
  thingspeak.setCallback(callback);
  Serial.println("Setup done");
}

void loop() {
  IAS.buttonLoop();

  if (millis() > iotEntry + 30000) {             // only for development. Please change it to at least 2 hours in production
    //  IAS.callHome();
    iotEntry = millis();
  }

  if (millis() > tempEntry + 5000) { // Wait a few seconds between measurements.
    tempEntry = millis();
    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    unsigned long readEntry = millis();
    do {
      h = dht.readHumidity();
      yield();
    } while (isnan(h) && millis() - readEntry < 500);
    // Read temperature as Celsius (the default)
    readEntry = millis();
    do {
      t = dht.readTemperature();
      yield();
    } while (isnan(t) && millis() - readEntry < 500);
    // Read temperature as Fahrenheit (isFahrenheit = true)
    readEntry = millis();
    do {
      f = dht.readTemperature(true);
      yield();
    } while (isnan(f) && millis() - readEntry < 500);

    // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t) || isnan(f)) {
      Serial.println("Failed to read from DHT sensor!");
      displayError();
    }
    else {
      // Compute heat index in Fahrenheit (the default)
      hif = dht.computeHeatIndex(f, h);
      // Compute heat index in Celsius (isFahreheit = false)
      hic = dht.computeHeatIndex(t, h, false);

      displayTemp(t);

      // Publish to Thingspeak
      publishToThingspeak();
      printLog();
    }
  }
}
