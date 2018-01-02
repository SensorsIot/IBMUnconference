/* DHT Pro Shield - Thingspeak

   Example testing sketch for various DHT humidity/temperature sensors
   Written by ladyada, public domain

   Depends on Adafruit DHT Arduino library
   https://github.com/adafruit/DHT-sensor-library
*/

#define APPNAME "WemosTempSensorThinspeak"
#define VERSION "V1.1.0"
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
  tempEntry = millis();
}

void setup() {
  IAS.serialdebug(true);                  // 1st parameter: true or false for serial debugging. Default: false

  IAS.preSetBoardname(APPNAME);
  IAS.preSetAutoUpdate(false);          
  IAS.preSetAutoConfig(false);           
  IAS.preSetWifi(mySSID, myPASSWORD);

  IAS.begin(true, 'P');

  IAS.onModeButtonConfigMode([]() {
    displayConfig();
  });

  IAS.onModeButtonFirmwareUpdate([]() {
    displayUpdatate();
  });

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

    // Publish to Thingspeak
    publishToThingspeak();
    printLog();
  }
}
