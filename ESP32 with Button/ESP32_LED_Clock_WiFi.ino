/*
   LED NeoPixel Digital Clock
   Code written by Alan Madorin

   23;   = NeoPixel Data Pin
   39;   = Push Button (Built-in)

   ESP32 Library        https://github.com/espressif/arduino-esp32
   FastLED Library      https://github.com/FastLED/FastLED
   ElegantOTA Library   https://github.com/ayushsharma82/ElegantOTA
   Time Library         https://github.com/PaulStoffregen/Time
*/

//##########################   Library/Variables   ###################################//

/***************************   WiFi  ********************************************/
#include <WiFi.h>
const char* ssid     = "SSID_GOES_HERE";     // SSID of Wifi
const char* password = "WIFI_PASSWORD_GOES_HERE";  // Wifi Password


/***************************   OTA   ********************************************/
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
AsyncWebServer server(80);


/***************************   HTTP Client  *************************************/
#include <HTTPClient.h>
#include <ArduinoJson.h>
String serverName = "https://worldtimeapi.org/api/timezone/America/Denver";  // Auto Adjust for Time Changes (SET CORRECT ONE FOR YOU http://worldtimeapi.org/)
byte hourNow = 0;
byte hourPrevious = 0;


/***************************   FastLED   ****************************************/
#include <FastLED.h>
#define DATA_PIN       23            // The Neopixel data pin
#define NUM_LEDS       130           // Number of Neopixel LED's connected
#define LED_TYPE       WS2812B       // Type of LED Chip
#define COLOR_ORDER    GRB           // Order of colors for LED Chip
#define BRIGHTNESS         255       // Max Brightness
#define FRAMES_PER_SECOND  120       // Used with FastLED.delay(1000/FRAMES_PER_SECOND);
CRGB leds[NUM_LEDS];

//  Red (0..) "HUE_RED", Orange (32..) "HUE_ORANGE", Yellow (64..) "HUE_YELLOW", Green (96..) "HUE_GREEN"
//  Aqua (128..) "HUE_AQUA", Blue (160..) "HUE_BLUE", Purple (192..) "HUE_PURPLE", Pink(224..) "HUE_PINK"

byte firstPixelHue = 0;                       // Rainbow starting color
byte brightnessLED = 128;                     // Adjustable by button
byte clockColor = 0;                          // Color for colorWipe & singleColor
const byte colorChangeAmount = 50;            // Jump between colors on colorWipe
byte colorWipePixel = 0;                      // Location of changing pixel color
byte colorWipeColor = 0 + colorChangeAmount;  // 2nd color in colorWipe that replaces 1st color
byte prevWipeColor = 0;                       // 1st color in colorWipe
// Variables for colors
int RedLED = 0;
int GreenLED = 180;
int BlueLED = 60;
byte Red_LED = 0;
byte Green_LED = 180;
byte Blue_LED = 60;


/***************************   Time  ********************************************/
#include <TimeLib.h>
#include <WiFiUdp.h>
static const char ntpServerName[] = "us.pool.ntp.org";
int timeZone = -7;                          // -7 = Mountain Standard Time (USA)  or  -6 = Mountain Daylight Time (USA)
WiFiUDP Udp;                                // UDP instance
unsigned const int localPort = 8888;        // local port to listen for UDP packets
time_t prevDisplay = 0;                     // when the digital clock was displayed
const int NTP_PACKET_SIZE = 48;             // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE];         // buffer to hold incoming & outgoing packets


/***************************   Button  ******************************************/
#define button 39
#define modeMax 19
#define modeMin 0
byte buttonPushCounter = 0;                 // counter for the number of button presses
unsigned long buttonTimer = 0;              // used to capture millis for button timer
const int longPressTime = 500;              // time to hold button for secondary action
boolean buttonActive = false;               // used for short or long press logic
boolean longPressActive = false;            // used for short or long press logic


/***************************   Variables  ***************************************/
const bool digit[15][21] =  {
  /*    1        2        3        4        5        6        7      */
  {0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, // 0
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0}, // 1
  {1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1}, // 2
  {1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0}, // 3
  {1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0}, // 4
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0}, // 5
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1}, // 6
  {0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0}, // 7
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, // 8
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0}, // 9
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // 10 = Blank
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1}, // 11 = F
  {1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1}, // 12 = H
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // 13 = upper circle
  {1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1}, // 14 = lower circle
};

bool pixelNumber[130];    // Used to capture each number and correctly align with pixel numbers

byte digitOne =  0;       // Variable to designate which number to show
byte digitTwo =  0;       // Variable to designate which number to show
byte digitThree =  0;     // Variable to designate which number to show
byte digitFour =  0;      // Variable to designate which number to show
byte digitFive =  0;      // Variable to designate which number to show
byte digitSix =  0;       // Variable to designate which number to show

unsigned long currentMillis = 0;   // Used throughout loop as current millis
byte currentSecond = 0;            // Used throughout loop as current second
byte currentMinute = 0;            // Used throughout loop as current minute


/***************************   EEPROM  ******************************************/
#include <EEPROM.h>
int addr = 0;           // address "0" is used for switch state; address "1" is used for brightness setting
// address "2" is for timezone offset


//##########################   Setup  ############################################//
void setup() {
  Serial.begin(115200);

  /***************************   EEPROM  ******************************************/
  EEPROM.begin(16);                       // Number of bytes allocated for EEPROM
  buttonPushCounter = EEPROM.read(addr);  // Sets switch state to previous state before restart
  addr++;                                 // Changes from 0 to 1
  brightnessLED = EEPROM.read(addr);      // Sets brightness to previous state before restart
  addr++;                                 // Changes from 1 to 2
  byte timeZoneByte = EEPROM.read(addr);  // Read timezone setting from before restart
  timeZone = timeZoneByte - 12;           // Adjust to actual value because byte is only positive
  updateColorLED();

  /***************************   Button  ******************************************/
  pinMode(button, INPUT);                 // Push button to control mode and brightness

  /***************************   FastLED   ****************************************/
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);   // tell FastLED about the LED strip configuration
  FastLED.setBrightness(BRIGHTNESS);                                  // Set master brightness control (max = 255)
  //for (byte i = 0; i < 130; i++) {                                    // For each pixel in strip...
  //  leds[i].setHSV(i+2, 255, 180);
  //}
  //FastLED.show();

  /***************************   WiFi  ********************************************/
  WiFi.setAutoReconnect (true);           // WiFi setting for auto reconnect (true = active)
  WiFi.mode(WIFI_STA);                    // Set WiFi to starting mode
  WiFi.begin(ssid, password);             // Initialize WiFi connection
  
  byte startLedNumber = 0;
  byte startLedColor = 0;
  
  while (WiFi.status() != WL_CONNECTED) { // While not connected, wait
    leds[startLedNumber].setHSV(startLedColor+2, 255, 180);
    FastLED.show();
    delay(100);
    startLedNumber++;
    startLedColor++;
    if (startLedNumber > 130) startLedNumber = 0;
  }
  
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  /***************************   OTA   ********************************************/
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain", "HI! This is the LED Clock.\n\n+++ Add /update to IP address to load the update webpage. +++");
  });
  AsyncElegantOTA.begin(&server);    // Start ElegantOTA
  server.begin();
  Serial.println("HTTP server started");

  /***************************   Time  ********************************************/
  Udp.begin(localPort);
  setSyncProvider(getNtpTime);
  setSyncInterval(60 * 15 * 1);     // seconds * minutes * hours

  timezoneManager();                // Call timezone update only once on boot
}    // end of setup()


//##########################   Loop   ############################################//

void loop() {
  currentSecond = second();
  currentMinute = minute();
  currentMillis = millis();
  buttonControl();
  
  /***************************   Clock Mode  **************************************/
  switch (buttonPushCounter) {
    case 0:
      runClock();
      singleColor(50);
      break;
    case 1:
      runClock();
      rainbow(10);
      break;
    case 2:
      runClock();
      colorWipe(25);
      break;
    case 3:
      runClock();
      twoColor(25, 128, 160);
      break;
    case 4:
      runClock();
      solidColor(25, 0, 255); // Red
      break;
    case 5:
      runClock();
      solidColor(25, 16, 255); // Red-Orange
      break;
    case 6:
      runClock();
      solidColor(25, 32, 255); // Orange
      break;
    case 7:
      runClock();
      solidColor(25, 48, 255); // Orange-Yellow
      break;
    case 8:
      runClock();
      solidColor(25, 64, 255); // Yellow
      break;
    case 9:
      runClock();
      solidColor(25, 80, 255); // Yellow-Green
      break;
    case 10:
      runClock();
      solidColor(25, 96, 255); // Green
      break;
    case 11:
      runClock();
      solidColor(25, 112, 255); // Green-Aqua
      break;
    case 12:
      runClock();
      solidColor(25, 128, 255); // Aqua
      break;
    case 13:
      runClock();
      solidColor(25, 144, 255); // Aqua-Blue
      break;
    case 14:
      runClock();
      solidColor(25, 160, 255); // Blue
      break;
    case 15:
      runClock();
      solidColor(25, 176, 255); // Blue-Purple
      break;
    case 16:
      runClock();
      solidColor(25, 192, 255); // Purple
      break;
    case 17:
      runClock();
      solidColor(25, 208, 255); // Purple-Pink
      break;
    case 18:
      runClock();
      solidColor(25, 224, 255); // Pink
      break;
    case 19:
      runClock();
      solidColor(25, 0, 0); // White
      break;
  }
}    // end of loop()


//##########################   Functions   #########################################//

/***************************   Button  ******************************************/
void buttonControl() {
  if (digitalRead(button) == LOW) {
    if (buttonActive == false) {
      buttonActive = true;
      buttonTimer = millis();
    }
    if ((millis() - buttonTimer > longPressTime) && (longPressActive == false)) {
      longPressActive = true;
      brightnessLED = brightnessLED - 32;                   // Brightness wraps around at 255
      addr = 1;                                             // Brightness address
      Serial.print("LED Brightness: ");
      Serial.println(brightnessLED);
      EEPROM.write(addr, brightnessLED);
      EEPROM.commit();
      updateColorLED();
    }
  } else {
    if (buttonActive == true) {
      if (longPressActive == true) {
        longPressActive = false;
      } else {
        buttonPushCounter++;
        if (buttonPushCounter > modeMax) buttonPushCounter = modeMin;   // Match number to the number of cases
        addr = 0;                                           // Button address
        Serial.print("Clock Mode: ");
        Serial.println(buttonPushCounter);
        EEPROM.write(addr, buttonPushCounter);
        EEPROM.commit();
        updateColorLED();
      }
      buttonActive = false;
    }
  }
}


/***************************   Clock  ********************************************/
void runClock() {
  if (timeStatus() != timeNotSet) {
    if (now() != prevDisplay) {                     //update the display only if time has changed
      prevDisplay = now();
      clockDigits();
      prepDigits();
    }
  }
}

void clockDigits() {
  digitOne = second() % 10;
  digitTwo = (second() - digitOne) / 10;
  digitThree = minute() % 10;
  digitFour = (minute() - digitThree) / 10;
  digitFive = hourFormat12() % 10;
  digitSix = (hourFormat12() - digitFive) / 10;
  if (digitSix == 0) {
    digitSix = 10;       // 10 is blank
  }

  pixelNumber[42] = 1;
  pixelNumber[43] = 1;
  pixelNumber[86] = 1;
  pixelNumber[87] = 1;
}


/***************************   Digits  ********************************************/
void prepDigits() {
  for (byte i = 0; i < 21; i++) {
    pixelNumber[i] = digit[digitOne][i];
  }
  for (byte i = 0; i < 21; i++) {
    pixelNumber[i + 21] = digit[digitTwo][i];
  }
  for (byte i = 0; i < 21; i++) {
    pixelNumber[i + 44] = digit[digitThree][i];
  }
  for (byte i = 0; i < 21; i++) {
    pixelNumber[i + 65] = digit[digitFour][i];
  }
  for (byte i = 0; i < 21; i++) {
    pixelNumber[i + 88] = digit[digitFive][i];
  }
  for (byte i = 0; i < 21; i++) {
    pixelNumber[i + 109] = digit[digitSix][i];
  }
}


/***************************   FastLED   ****************************************/
// Input is a delay time (in milliseconds) between pixels.

void colorWipe(int wait) {                          // Fill strip pixels one after another with a color. Strip is NOT cleared first;
  for (byte i = 0; i < 130; i++) {                  //For each pixel in strip...
    if (i > colorWipePixel) {
      clockColor = prevWipeColor;
    } else {
      clockColor = colorWipeColor;
    }
    leds[i].setHSV(clockColor, 255, (brightnessLED * pixelNumber[i]));
  }
  FastLED.show();                                   //Update strip to match
  colorWipePixel++;
  if (colorWipePixel > 130) {
    colorWipePixel = 0;
    colorWipeColor = colorWipeColor + colorChangeAmount;
    prevWipeColor = prevWipeColor + colorChangeAmount;
  }
  delay(wait);
}

void rainbow(int wait) {                            // Rainbow cycle along whole strip. Pass delay time (in ms) between frames.
  for (byte i = 0; i < 130; i++) {                  // For each pixel in strip...
    int pixelHue = firstPixelHue + (i * 2);
    leds[i].setHSV(pixelHue, 255, (brightnessLED * pixelNumber[i]));
  }
  FastLED.show();                                   // Update strip with new contents
  delay(wait);
  firstPixelHue = firstPixelHue + 1;
  if (firstPixelHue >= 255) firstPixelHue = 0;
}

void singleColor(int wait) {                        // Entire clock fades the colors of the rainbow.
  for (byte i = 0; i < 130; i++) {                  // For each pixel in strip...
    leds[i].setHSV(clockColor, 255, (brightnessLED * pixelNumber[i]));
  }
  FastLED.show();

  clockColor = clockColor + 1;
  if (clockColor >= 255) clockColor = 0;
  delay(wait);
}

void solidColor(int wait, byte solidColor, byte solidSaturation) {                         // Entire clock stays one color.
  for (byte i = 0; i < 130; i++) {                  // For each pixel in strip...
    leds[i].setHSV(solidColor, solidSaturation, (brightnessLED * pixelNumber[i]));
  }
  FastLED.show();
  delay(wait);
}

void updateColorLED() {
  if (buttonPushCounter == 4) {
    FastLED.setBrightness(brightnessLED);
  } else {
    FastLED.setBrightness(BRIGHTNESS);
  }
  float brightnessRGB = brightnessLED;
  brightnessRGB = brightnessRGB / (RedLED + GreenLED + BlueLED);
  Red_LED = (RedLED * brightnessRGB);
  Green_LED = (GreenLED * brightnessRGB);
  Blue_LED = (BlueLED * brightnessRGB);
}

void twoColor(int wait, byte colorOne, byte colorTwo) {
  if (digitOne % 2) {
    for (byte i = 0; i < 21; i++) {                  // For each pixel in strip...
      leds[i].setHSV(colorOne, 255, (brightnessLED * pixelNumber[i]));
    }
    for (byte i = 0; i < 21; i++) {                  // For each pixel in strip...
      leds[i + 21].setHSV(colorTwo, 255, (brightnessLED * pixelNumber[i + 21]));
    }
    for (byte i = 0; i < 21; i++) {                  // For each pixel in strip...
      leds[i + 44].setHSV(colorOne, 255, (brightnessLED * pixelNumber[i + 44]));
    }
    for (byte i = 0; i < 21; i++) {                  // For each pixel in strip...
      leds[i + 65].setHSV(colorTwo, 255, (brightnessLED * pixelNumber[i + 65]));
    }
    for (byte i = 0; i < 21; i++) {                  // For each pixel in strip...
      leds[i + 88].setHSV(colorOne, 255, (brightnessLED * pixelNumber[i + 88]));
    }
    for (byte i = 0; i < 21; i++) {                  // For each pixel in strip...
      leds[i + 109].setHSV(colorTwo, 255, (brightnessLED * pixelNumber[i + 109]));
    }

  } else {
    for (byte i = 0; i < 21; i++) {                  // For each pixel in strip...
      leds[i].setHSV(colorTwo, 255, (brightnessLED * pixelNumber[i]));
    }
    for (byte i = 0; i < 21; i++) {                  // For each pixel in strip...
      leds[i + 21].setHSV(colorOne, 255, (brightnessLED * pixelNumber[i + 21]));
    }
    for (byte i = 0; i < 21; i++) {                  // For each pixel in strip...
      leds[i + 44].setHSV(colorTwo, 255, (brightnessLED * pixelNumber[i + 44]));
    }
    for (byte i = 0; i < 21; i++) {                  // For each pixel in strip...
      leds[i + 65].setHSV(colorOne, 255, (brightnessLED * pixelNumber[i + 65]));
    }
    for (byte i = 0; i < 21; i++) {                  // For each pixel in strip...
      leds[i + 88].setHSV(colorTwo, 255, (brightnessLED * pixelNumber[i + 88]));
    }
    for (byte i = 0; i < 21; i++) {                  // For each pixel in strip...
      leds[i + 109].setHSV(colorOne, 255, (brightnessLED * pixelNumber[i + 109]));
    }
  }

  leds[42].setHSV(0, 0, brightnessLED);
  leds[43].setHSV(0, 0, brightnessLED);
  leds[86].setHSV(0, 0, brightnessLED);
  leds[87].setHSV(0, 0, brightnessLED);

  FastLED.show();
  delay(wait);
}


/***************************   WiFi  ********************************************/
bool wifiConnection() {
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.disconnect();
    delay(100);
    Serial.println("Wifi Reconnecting");
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    delay(200);
    if (WiFi.status() == WL_CONNECTED) {
      return 1;
    } else {
      return 0;
    }
  } else {
    return 1;
  }
}


/***************************   NTP  *********************************************/
time_t getNtpTime() {
  if (wifiConnection()) {
    IPAddress ntpServerIP;                          // NTP server's ip address
    while (Udp.parsePacket() > 0) yield();          // discard any previously received packets
    WiFi.hostByName(ntpServerName, ntpServerIP);    // get a random server from the pool
    sendNTPpacket(ntpServerIP);
    uint32_t beginWait = millis();
    while (millis() - beginWait < 1500) {
      yield();
      int size = Udp.parsePacket();
      if (size >= NTP_PACKET_SIZE) {
        Udp.read(packetBuffer, NTP_PACKET_SIZE);    // read packet into the buffer
        unsigned long secsSince1900;                // convert four bytes starting at location 40 to a long integer
        secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
        secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
        secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
        secsSince1900 |= (unsigned long)packetBuffer[43];
        return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
      }
    }
  }
  return 0;                                        // return 0 if unable to get the time
}

void sendNTPpacket(IPAddress &address)             // send an NTP request to the time server at the given address
{
  memset(packetBuffer, 0, NTP_PACKET_SIZE);        // Set all bytes in the buffer to 0
  packetBuffer[0] = 0b11100011;                    // LI, Version, Mode
  packetBuffer[1] = 0;                             // Stratum, or type of clock
  packetBuffer[2] = 6;                             // Polling Interval
  packetBuffer[3] = 0xEC;                          // Peer Clock Precision
  packetBuffer[12] = 49;                           // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;                           // All NTP fields have been given values, now
  Udp.beginPacket(address, 123);                   // NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
  Serial.println("Time Updated");
}


/***************************  HTTP GET TIMEZONE  ********************************/
void timezoneManager() {
  if (wifiConnection()) {
    bool timezoneBool = httpGetPayload();
    byte timeZoneByte = 0;

    if (timezoneBool == false) {
      timeZone = -7;                    // -7 = Mountain Standard Time (USA)
      timeZoneByte = timeZone + 12;
      EEPROM.write(2, timeZoneByte);
      EEPROM.commit();
      Serial.println("Timezone Updated to -7 Mountain Standard Time (USA)");
    
    } else if (timezoneBool == true) {
      timeZone = -6;                    // -6 = Mountain Daylight Time (USA)
      timeZoneByte = timeZone + 12;
      EEPROM.write(2, timeZoneByte);
      EEPROM.commit();
      Serial.println("Timezone Updated to -6 = Mountain Daylight Time (USA)");     
    }    
  } else {
      byte timeZoneByte = EEPROM.read(3);  // Read timezone setting from before restart
      timeZone = timeZoneByte - 12;   
  }
}

bool httpGetPayload() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    // Send request
    http.useHTTP10(true);
    http.begin(serverName.c_str());
    int httpResponseCode = http.GET();

    StaticJsonDocument<256> doc;
    deserializeJson(doc, http.getStream());

    /*    DeserializationError error = deserializeJson(doc, http.getStream());

        if (error) {
          Serial.print(F("deserializeJson() failed: "));
          Serial.println(error.f_str());
          return;
        }
    */
    //const char* abbreviation = doc["abbreviation"]; // "MDT"
    //const char* client_ip = doc["client_ip"]; // "24.11.53.227"
    //const char* datetime = doc["datetime"]; // "2021-07-20T20:22:57.419423-06:00"
    //int day_of_week = doc["day_of_week"]; // 2
    //int day_of_year = doc["day_of_year"]; // 201
    bool dst = doc["dst"]; // true
    //const char* dst_from = doc["dst_from"]; // "2021-03-14T09:00:00+00:00"
    //int dst_offset = doc["dst_offset"]; // 3600
    //const char* dst_until = doc["dst_until"]; // "2021-11-07T08:00:00+00:00"
    //int raw_offset = doc["raw_offset"]; // -25200
    //const char* timezone = doc["timezone"]; // "America/Denver"
    //long unixtime = doc["unixtime"]; // 1626834177
    //const char* utc_datetime = doc["utc_datetime"]; // "2021-07-21T02:22:57.419423+00:00"
    //const char* utc_offset = doc["utc_offset"]; // "-06:00"
    //int week_number = doc["week_number"]; // 29

    // Disconnect
    http.end();

    return dst;
  }
}
