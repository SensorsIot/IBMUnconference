/* Wemos - Lightbulb

   Example testing sketch for various DHT humidity/temperature sensors
   Written by ladyada, public domain

   Depends on Adafruit DHT Arduino library
   https://github.com/adafruit/DHT-sensor-library
*/

#define APPNAME "WemosLight"
#define VERSION "V1.0.0"
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
#define COMMANDTOPIC "light/command"
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

void displayPublished(String message) {
  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(32, 15, message.c_str());
  display.setFont(ArialMT_Plain_16);
  display.drawString(32, 40, "");
  display.display();
}

void callback(char* topic, byte* payload, unsigned int length) {
  String MACaddress = "";
  for (int i = 0; i < length; i++) {
    MACaddress += (char)payload[i];
  }
  displayMAC(MACaddress);
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print(" ");
  Serial.print(MACaddress);
  Serial.println("] ");

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
  IAS.serialdebug(true);

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
  display.init();
  display.flipScreenVertically();

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
    else payload = "ON";
    publishToMQTTbroker(COMMANDTOPIC, payload.c_str());
    displayPublished(payload);
  }
  lastButtonState = buttonState;
}
