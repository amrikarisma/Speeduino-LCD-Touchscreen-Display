#include "Arduino.h"
#include "SPI.h"
#include <TFT_eSPI.h> // Library TFT_eSPI

#include "NotoSansBold15.h"
#include "NotoSansBold36.h"

#define AA_FONT_SMALL NotoSansBold15
#define AA_FONT_LARGE NotoSansBold36

TFT_eSPI display = TFT_eSPI(); // Create TFT instance
TFT_eSprite spr    = TFT_eSprite(&display);

#include "Comms.h"
#include "text_utils.h"
#include "drawing_utils.h"

#define UART_BAUD 115200
#define RXD 3
#define TXD 1

boolean sent = false;
boolean received = true;
bool firstLoop = true;

uint8_t iat, clt;
uint8_t refreshRate;
unsigned int rpm, lastRpm;
int mapData, tps, adv;
float bat, afrConv;
bool syncStatus, fan, ase, wue, rev, launch, airCon;

int lastIat = -1, lastClt = -1, lastTps = -1, lastAdv = -1, lastMapData = -1;
float lastBat = -1, lastAfrConv = -1;
unsigned int lastRefreshRate = -1;

uint32_t lazyUpdateTime;
uint16_t  spr_width = 0;

void setup() {
  display.init();
  display.setRotation(1);
  display.fillScreen(TFT_BLACK);


  Serial.begin(UART_BAUD);
  delay(500);

  display.loadFont(AA_FONT_SMALL);
  spr.setColorDepth(16); 

  display.setTextColor(TFT_WHITE, TFT_BLACK);
  display.drawString("MAZDUINO_gank", 160, 5);
  display.setTextColor(TFT_WHITE, TFT_BLACK);
  display.drawString("RPM", 190, 120);
}

void loop() {
  static uint32_t lastUpdate = millis();
  if (millis() - lastUpdate > 10) {
    requestData(50);
    lastUpdate = millis();
  }

  static uint32_t lastRefresh = millis();
  uint32_t elapsed = millis() - lastRefresh;
  refreshRate = (elapsed > 0) ? (1000 / elapsed) : 0;
  lastRefresh = millis();
  if (lastRefresh - lazyUpdateTime > 1000) {
      clt = getByte(7) - 40;
      iat = getByte(6) - 40;
      bat = getByte(9);
  }
  // Data acquisition
  rpm = getWord(14);
  mapData = getWord(4);
  afrConv = getByte(10);
  tps = getByte(24) / 2;
  adv = (int8_t)getByte(23);
  syncStatus = getBit(31, 7);
  ase = getBit(2, 2);
  wue = getBit(2, 3);
  rev = getBit(31, 2);
  launch = getBit(31, 0);
  airCon = getByte(122);
  fan = getBit(106, 3);
  drawData();
}


void drawDataBox(int x, int y, const char* label, const int value, uint16_t labelColor, const int valueToCompare, const int decimal) {
  const int BOX_WIDTH = 110;  // Reduced width to fit screen
  const int BOX_HEIGHT = 80; // Adjusted height

  // Check if value is different from previous value
  if (valueToCompare != value) {
    const int LABEL_HEIGHT = BOX_HEIGHT / 2;
    if (firstLoop) {
      spr.loadFont(AA_FONT_SMALL); 
      spr.createSprite(BOX_WIDTH, LABEL_HEIGHT);
      spr.setTextColor(labelColor, TFT_BLACK, true);
      spr.drawString(label, 50, 5);
      spr.setTextDatum(TC_DATUM);
      spr.pushSprite(x, y);
    }
    
    spr.loadFont(AA_FONT_LARGE);
    spr.createSprite(BOX_WIDTH, LABEL_HEIGHT);
    spr.setTextDatum(TC_DATUM);
    spr_width = spr.textWidth("333"); 
    spr.setTextColor(labelColor, TFT_BLACK, true);
    if (decimal > 0) {
      spr.drawFloat((value/10.0),decimal,  50, 5);
    } else {
      spr.drawNumber(value,  50, 5);
    }
    spr.pushSprite(x, y + LABEL_HEIGHT - 15);
    spr.deleteSprite();
  }
}

void drawData() {
  if (lastRpm != rpm) {
    drawRPMBarBlocks(rpm);
    spr.loadFont(AA_FONT_LARGE);
    spr.createSprite(100, 50);
    spr_width = spr.textWidth("7777"); // 7 is widest numeral in this font
    spr.setTextColor(TFT_WHITE, TFT_BLACK, true);
    spr.setTextDatum(TR_DATUM);
    spr.drawNumber(rpm,  100, 5);
    spr.pushSprite(190, 140);
    spr.deleteSprite();
    lastRpm = rpm;
  }



  const char* labels[] = {"AFR", "TPS", "ADV", "MAP"};
  int values[] = {afrConv, tps, adv, mapData};
  int lastValues[] = {lastAfrConv, lastTps, lastAdv, lastMapData};
  int positions[][2] = {{5, 190}, {360, 190}, {120, 190}, {360, 10}};
  uint16_t colors[] = {(afrConv < 130) ? TFT_ORANGE : (afrConv > 14.7) ? TFT_RED : TFT_GREEN, TFT_WHITE, TFT_RED, TFT_WHITE};

  for (int i = 0; i < 4; i++) {
    drawDataBox(positions[i][0], positions[i][1], labels[i],  values[i], colors[i], lastValues[i], ( i == 0) ? 1 : 0);
    lastValues[i] = values[i];
  }
  if (millis() - lazyUpdateTime > 1000) {
    const char* labelsLazy[] = {"IAT", "Coolant", "Voltage", "FPS"};
    int valuesLazy[] = {iat, clt, static_cast<int>(bat), refreshRate};
    int lastValuesLazy[] = {lastIat, lastClt, static_cast<int>(lastBat), lastRefreshRate};
    int positionsLazy[][2] = {{5, 10}, {5, 100}, {360, 100}, {240, 190}};
    uint16_t colorsLazy[] = {TFT_WHITE, (clt > 95) ? TFT_RED : TFT_WHITE, 
                             (bat < 11.5 || bat > 14.5) ? TFT_ORANGE : TFT_GREEN, TFT_WHITE};

    for (int i = 0; i < 4; i++) {
      drawDataBox(positionsLazy[i][0], positionsLazy[i][1], labelsLazy[i], valuesLazy[i], colorsLazy[i], lastValuesLazy[i], ( i == 2) ? 1 : 0);
      lastValuesLazy[i] = valuesLazy[i];
    }
    lazyUpdateTime = millis();
    firstLoop = false;
  }
  // Center buttons
  display.loadFont(AA_FONT_SMALL);
  const char* buttonLabels[] = {"SYNC", "FAN", "ASE", "WUE", "REV", "LCH", "AC"};
  bool buttonStates[] = {syncStatus, fan, ase, wue, rev, launch, airCon};
  for (int i = 0; i < 7; i++) {
    drawSmallButton(10 + 70 * i, 280, buttonLabels[i], buttonStates[i]);
  }

}