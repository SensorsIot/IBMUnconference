/* This is an initial sketch to be used as a "blueprint" to create apps which can be used with IOTappstory.com infrastructure
  Your code can be filled wherever it is marked.


  Copyright (c) [2016] [Andreas Spiess]

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

*/

#define APPNAME "WemosClock"
#define VERSION "V2.1.0"
#define COMPDATE __DATE__ __TIME__
#define MODEBUTTON D3


#include <IOTAppStory.h>
#include <SNTPtime.h>
#include <SSD1306.h>
#include <credentials.h>

#ifndef CREDENTIALS
#define mySSID "*****"
#define myPASSWORD "*****"
#endif

SNTPtime NTPch("ch.pool.ntp.org");
IOTAppStory IAS(APPNAME, VERSION, COMPDATE, MODEBUTTON);
SSD1306  display(0x3c, D2, D1);

strDateTime dateTime;

int screenW = 64;
int screenH = 48;
int clockCenterX = screenW / 2;
int clockCenterY = ((screenH - 16) / 2) + 16; // top yellow part is 16 px height
int clockRadius = 23;
int x = 30, y = 10;

char* timeZone = "1.0";
int lastSecond;
unsigned long iotEntry = millis();

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

// ================================================ SETUP ================================================
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

  while (!NTPch.setSNTPtime()) Serial.print("."); // set internal clock
  Serial.println();
  Serial.println("Time set");
}


// ================================================ LOOP =================================================
void loop() {
  IAS.buttonLoop();
  dateTime = NTPch.getTime(1.0, 1); // get time from internal clock
  if (dateTime.second != lastSecond) {
    NTPch.printDateTime(dateTime);
    drawFace();
    drawArms(dateTime.hour, dateTime.minute, dateTime.second);
    display.display();
    lastSecond = dateTime.second;
  }
}

// Draw an analog clock face
void drawFace() {
  display.clear();
  display.drawCircle(clockCenterX + x, clockCenterY + y, 2);
  //
  //hour ticks
  for ( int z = 0; z < 360; z = z + 30 ) {
    //Begin at 0° and stop at 360°
    float angle = z ;
    angle = ( angle / 57.29577951 ) ; //Convert degrees to radians
    int x2 = ( clockCenterX + ( sin(angle) * clockRadius ) );
    int y2 = ( clockCenterY - ( cos(angle) * clockRadius ) );
    int x3 = ( clockCenterX + ( sin(angle) * ( clockRadius - ( clockRadius / 8 ) ) ) );
    int y3 = ( clockCenterY - ( cos(angle) * ( clockRadius - ( clockRadius / 8 ) ) ) );
    display.drawLine( x2 + x , y2 + y , x3 + x , y3 + y);
  }
}

// Draw the clock's three arms: seconds, minutes, hours.
void drawArms(int h, int m, int s)
{
  // display second hand
  float angle = s * 6 ;
  angle = ( angle / 57.29577951 ) ; //Convert degrees to radians
  int x3 = ( clockCenterX + ( sin(angle) * ( clockRadius - ( clockRadius / 5 ) ) ) );
  int y3 = ( clockCenterY - ( cos(angle) * ( clockRadius - ( clockRadius / 5 ) ) ) );
  display.drawLine( clockCenterX + x , clockCenterY + y , x3 + x , y3 + y);
  //
  // display minute hand
  angle = m * 6 ;
  angle = ( angle / 57.29577951 ) ; //Convert degrees to radians
  x3 = ( clockCenterX + ( sin(angle) * ( clockRadius - ( clockRadius / 4 ) ) ) );
  y3 = ( clockCenterY - ( cos(angle) * ( clockRadius - ( clockRadius / 4 ) ) ) );
  display.drawLine( clockCenterX + x , clockCenterY + y , x3 + x , y3 + y);
  //
  // display hour hand
  angle = h * 30 + int( ( m / 12 ) * 6 )   ;
  angle = ( angle / 57.29577951 ) ; //Convert degrees to radians
  x3 = ( clockCenterX + ( sin(angle) * ( clockRadius - ( clockRadius / 2 ) ) ) );
  y3 = ( clockCenterY - ( cos(angle) * ( clockRadius - ( clockRadius / 2 ) ) ) );
  display.drawLine( clockCenterX + x , clockCenterY + y , x3 + x , y3 + y);
}
