#include "Arduino.h"
#include <MCUFRIEND_kbv.h>
MCUFRIEND_kbv display;       // hard-wired for UNO / MEGA shields anyway.
#include "Comms.h"

#include "colors.h"
#include "text_utils.h"
#include "drawing_utils.h"

#define UART_BAUD 115200
#define packTimeout 1
#define bufferSize 8192

uint16_t ID;
uint32_t lazyUpdateTime;
uint8_t readiat; // clt doesn't need to be updated very ofter so
int iat;   // to store coolant temp
uint8_t readclt; // clt doesn't need to be updated very ofter so
int clt;   // to store coolant temp
int tps;
float bat;
int adv;
unsigned int rpm = 0;  //rpm and PW from speeduino
unsigned int lastRpm = 0;
int mapData;
float afrConv;
bool syncStatus;
bool fan;
bool ase;
bool wue;
bool rev;
bool launch;
bool airCon; 

void setup () 
{
  display.reset();
  ID = display.readID();
  display.begin(ID);
  display.setRotation(1);
  display.fillScreen(BLACK);

  Serial.begin(UART_BAUD);
  delay(500);

  display.setTextSize(2);
  display.setCursor(150, 5);
  display.print("MAZDUINO_gank");

  display.fillRect(185, 138, 130, 40, BLACK);
  display.setTextSize(2);
  display.setTextColor(WHITE, BLACK);
  display.setCursor(190, 120);
  display.print("RPM");
  display.setCursor(190, 140);
  display.setTextSize(4);
  display.print("0");

  drawRPMBarBlocks(0);

  }

boolean sent = false;
boolean received = true;
uint32_t sendTimestamp;
uint16_t refreshRate;

void loop () {
  // serial operation, frequency based request
  static uint32_t lastUpdate = millis();
  if (millis() - lastUpdate > 10)
  {
    requestData(50);
    lastUpdate = millis();
  }

  // get refresh rate
  static uint32_t lastRefresh = millis();
  refreshRate = 1000 / (millis() - lastRefresh);
  lastRefresh = millis();
  if(millis() - lazyUpdateTime > 2000) {
    bat =  getByte(9);
    iat = getByte(6)-40;
    clt = (int16_t)getByte(7)-40;
  }
  rpm = getWord(14); 
  mapData = getWord(4);
  afrConv = getByte(10);
  tps = getByte(24)/2;
  adv = (int8_t)getByte(23);
  syncStatus =   getBit(31, 7);
  ase = getBit(2, 2);
  wue = getBit(2, 3);
  rev = getBit(31, 2);
  launch = getBit(31, 0);
  airCon = getByte(122)/10;
  fan = getBit(106, 3);;
  drawData();
}


void drawDataBox(int x, int y, const char* label, const char* value, uint16_t labelColor) {
  const int BOX_WIDTH = 110;  // Reduced width to fit screen
  const int BOX_HEIGHT = 80; // Adjusted height

  drawRoundedBox(x, y, BOX_WIDTH, BOX_HEIGHT, CYAN);
  
  const int LABEL_HEIGHT = BOX_HEIGHT / 2;
  drawCenteredText(x, y, BOX_WIDTH, LABEL_HEIGHT, label, 2, labelColor);
  
  drawCenteredText(x, y + LABEL_HEIGHT, BOX_WIDTH, LABEL_HEIGHT, value, 3, labelColor);
}

void drawSmallDataBox(int x, int y, const char* label, const char* value, uint16_t labelColor) {
  const int BOX_WIDTH = 60;  // Reduced width to fit screen
  const int BOX_HEIGHT = 20; // Adjusted height

  const int LABEL_HEIGHT = BOX_HEIGHT / 1;
  sprintf(buffer, "%s%s", label, value);
  drawCenteredText(x, y, BOX_WIDTH, LABEL_HEIGHT, buffer, 1, labelColor);

}

void drawData() {

  char valueBuffer[5]; 

  // Left Column
  if(millis() - lazyUpdateTime > 2000) {
    formatValue(valueBuffer, refreshRate, 0);
    drawSmallDataBox(0, 0, "FPS: ", valueBuffer, WHITE);
    
    // IAT
    formatValue(valueBuffer, iat, 0);
    drawDataBox(5, 10, "IAT", valueBuffer, WHITE);

    // Coolant
    formatValue(valueBuffer, clt, 0);

    if (clt > 135) {
      drawDataBox(5, 100, "Coolant", valueBuffer, RED);
    } else {
      drawDataBox(5, 100, "Coolant", valueBuffer, WHITE);
    }
  }

  // AFR
  formatValue(valueBuffer, afrConv, 1);
  if (afrConv<13 ) {
    drawDataBox(5, 190, "AFR", valueBuffer, ORANGE);
  } else if(afrConv>14.8) {
    drawDataBox(5, 190, "AFR", valueBuffer, RED);
  } else {
    drawDataBox(5, 190, "AFR", valueBuffer, GREEN);
  }

  // Center Column
  // RPM
  if (lastRpm != rpm) {
    display.fillRect(185, 138, 130, 40, BLACK);
    display.setTextSize(2);
    display.setTextColor(WHITE, BLACK);
    display.setCursor(190, 120);
    display.print("RPM");
    display.setCursor(190, 140);
    display.setTextSize(4);
    formatValue(valueBuffer, rpm, 0);
    display.print(valueBuffer);

    drawRPMBarBlocks(rpm);

    lastRpm = rpm;
  }


  // Center buttons
  drawSmallButton(10, 280, "SYNC", syncStatus);
  drawSmallButton(80, 280, "FAN", fan);
  drawSmallButton(150, 280, "ASE", ase);
  drawSmallButton(220, 280, "WUE", wue);
  drawSmallButton(290, 280, "REV", rev);
  drawSmallButton(360, 280, "LCH", launch);
  drawSmallButton(430, 280, "AC", airCon);

  // ADVANCE
  formatValue(valueBuffer, adv, 0);
  drawDataBox(185, 190, "ADV", valueBuffer, RED);

  // Right Column
  // MAP
  formatValue(valueBuffer, mapData, 0);
  drawDataBox(360, 10, "MAP", valueBuffer, WHITE);

  // Voltage
  if(millis() - lazyUpdateTime > 2000) {
    formatValue(valueBuffer, bat, 1);
    if (bat<11.5 | bat > 14.5) {
      drawDataBox(360, 100, "Voltage", valueBuffer, ORANGE);
    } else {
      drawDataBox(360, 100, "Voltage", valueBuffer, GREEN);
    }
    lazyUpdateTime = millis();
  }

  // TPS
  formatValue(valueBuffer, tps, 0);
  drawDataBox(360, 190, "TPS", valueBuffer, WHITE);

}