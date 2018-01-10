/* Wemos - Lightbulb

   Example testing sketch for various DHT humidity/temperature sensors
   Written by ladyada, public domain

   Depends on Adafruit DHT Arduino library
   https://github.com/adafruit/DHT-sensor-library
*/

#define APPNAME "WemosLight"
#define VERSION "V2.1.1"
#define COMPDATE __DATE__ __TIME__
#define MODEBUTTON D3

#include <SSD1306.h>
#include <ESP8266WiFi.h>
#include <IOTAppStory.h>
#include <PubSubClient.h>
#include <credentials.h>

#ifndef CREDENTIALS
#define mySSID "*****"
#define myPASSWORD "*****"
#endif

const char *mqtt_server = "m23.cloudmqtt.com";
const int mqtt_port = 10750;
const char *mqtt_user = "fiqpqewg";
const char *mqtt_pass = "8ye0CMEWL85c";
const char *mqtt_client_name = "IBMunconference"; // Client connections cant have the same connection name

#define SERVICETOPIC "light/service"
#define COMMANDTOPIC "light/command/relay/0/set"
String payload;
bool buttonState = false, lastButtonState;
bool bulb = false;

WiFiClient espClient;
PubSubClient MQTTbroker(espClient);
IOTAppStory IAS(APPNAME, VERSION, COMPDATE, MODEBUTTON);
SSD1306  display(0x3c, D2, D1);

int tempEntry;
unsigned long iotEntry = millis();

void displayMAC(String MACaddress) {
  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(32, 15, "Other");
  display.setFont(ArialMT_Plain_10);
  display.drawString(32, 35, MACaddress.substring(0, 8));
  display.drawString(32, 50, MACaddress.substring(9, 17));
  display.display();
}

void displayPublished(String message) {
  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(32, 15, message.c_str());
  display.setFont(ArialMT_Plain_16);
  display.drawString(32, 40, "");
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
  display.drawString(32, 13, F("Update"));
  display.drawString(52, 31, F("of"));
  display.drawString(32, 49, F("Sketch"));
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
  String MACaddress = "";
  for (int i = 0; i < length; i++) {
    MACaddress += (char)payload[i];
  }
  Serial.println(MACaddress);
  Serial.println(WiFi.macAddress());
  Serial.println(MACaddress == WiFi.macAddress());
  Serial.println(MACaddress.equals(WiFi.macAddress()));

  if (MACaddress != WiFi.macAddress()) {  // do not display own MAC Address
    displayMAC(MACaddress);
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print(" ");
    Serial.print(MACaddress);
    Serial.println("] ");
  }
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
  while (!MQTTbroker.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    String boardName = APPNAME"_" + WiFi.macAddress();
    if (MQTTbroker.connect(boardName.c_str(), mqtt_user, mqtt_pass)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      MQTTbroker.publish( SERVICETOPIC, WiFi.macAddress().c_str());
      // ... and resubscribe
      MQTTbroker.subscribe(SERVICETOPIC);
    } else {
      Serial.print("failed, rc=");
      Serial.print(MQTTbroker.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void publishToMQTTbroker(String topic, String payload) {
  if (!MQTTbroker.connected()) {
    reconnect();
  }
  MQTTbroker.loop();
  Serial.print("Publish message: ");
  Serial.println(payload);
  MQTTbroker.publish(topic.c_str(), payload.c_str());
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
  IAS.preSetAutoConfig(false);
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

  MQTTbroker.setServer(mqtt_server, mqtt_port);
  MQTTbroker.setCallback(callback);
}

void loop() {
  IAS.buttonLoop();
  MQTTbroker.loop();
  bool stateChanged = false;
  buttonState = digitalRead(MODEBUTTON);


  if (buttonState == true && lastButtonState == false)  {
    Serial.println(buttonState);
    Serial.println(lastButtonState);
    stateChanged = true;
  }

  if (stateChanged) {
    bulb = !bulb;

    if (bulb) payload = "OFF";
    else {
      MQTTbroker.publish( SERVICETOPIC, WiFi.macAddress().c_str());
      payload = "ON";
    }
    publishToMQTTbroker(COMMANDTOPIC, payload.c_str());
    displayPublished(payload);
  }
  lastButtonState = buttonState;
}
