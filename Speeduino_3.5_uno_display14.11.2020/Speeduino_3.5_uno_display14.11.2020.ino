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

uint8_t readiat; // clt doesn't need to be updated very ofter so
int iat;   // to store coolant temp
uint8_t readclt; // clt doesn't need to be updated very ofter so
int clt;   // to store coolant temp
int tps;
float bat;
int adv;
unsigned int rpm;  //rpm and PW from speeduino
unsigned int lastRpm;
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

  }

float rps;
boolean sent = false;
boolean received = true;
uint32_t sendTimestamp;

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
  uint16_t refreshRate = 1000 / (millis() - lastRefresh);
  lastRefresh = millis();
  rpm = getWord(14); // rpm low & high (Int) TBD: probaply no need to split high and low bytes etc. this could be all simpler
  mapData = getWord(4);
  clt = (int16_t)getByte(7)-40;
  afrConv = getByte(10);
  iat = getByte(6)-40;
  tps = getByte(25)/2;
  bat =  getByte(9);
  adv = (int8_t)getByte(24);
  syncStatus =   getBit(31, 7);
  ase = getBit(2, 2);
  wue = getBit(2, 3);
  rev = getBit(31, 2);
  launch = getBit(31, 0);
  airCon = getByte(122)/10;
  fan = getBit(106, 3);;
  lastRpm = rpm;
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

void drawData() {
  // display.fillScreen(BLACK); // Dark blue background

  char valueBuffer[22]; // Buffer for converting numbers to strings
  display.setTextSize(2);
  display.setCursor(150, 5);
  display.print("MAZDUINO_gank");

  drawRPMBarBlocks(rpm);

  // Left Column
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
  display.fillRect(185, 138, 130, 40, BLACK);
  display.setTextSize(2);
  display.setTextColor(WHITE, BLACK);
  display.setCursor(190, 120);
  display.print("RPM");
  display.setCursor(190, 140);
  display.setTextSize(4);
  formatValue(valueBuffer, rpm, 0);
  display.print(valueBuffer);

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
  formatValue(valueBuffer, bat, 1);
  if (bat<11.5 | bat > 14.5) {
    drawDataBox(360, 100, "Voltage", valueBuffer, ORANGE);
  } else {
    drawDataBox(360, 100, "Voltage", valueBuffer, GREEN);
  }
  // TPS
  formatValue(valueBuffer, tps, 0);
  drawDataBox(360, 190, "TPS", valueBuffer, WHITE);

  delay(40);
}