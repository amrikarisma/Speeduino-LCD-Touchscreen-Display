#include "Arduino.h"
#include "SPI.h"
#include <TFT_eSPI.h> // Library TFT_eSPI

#include "NotoSansBold15.h"
#include "NotoSansBold36.h"

#define AA_FONT_SMALL NotoSansBold15
#define AA_FONT_LARGE NotoSansBold36

TFT_eSPI display = TFT_eSPI(); // Create TFT instance

#include "Comms.h"
#include "text_utils.h"
#include "drawing_utils.h"

#define UART_BAUD 115200
#define RXD 16
#define TXD 17

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

void setup() {
  display.init();
  display.setRotation(1);
  display.fillScreen(TFT_BLACK);
  display.loadFont(AA_FONT_SMALL);
  Serial.begin(UART_BAUD);
  Serial1.begin(UART_BAUD, SERIAL_8N1, RXD, TXD);
  delay(500);
}

void loop() {
  static uint32_t lastUpdate = millis();
  if (millis() - lastUpdate > 20) {
    requestData(50);
    lastUpdate = millis();
  }

  static uint32_t lastRefresh = millis();
  uint32_t elapsed = millis() - lastRefresh;
  refreshRate = (elapsed > 0) ? (1000 / elapsed) : 0;
  lastRefresh = millis();

  // Data acquisition
  rpm = getWord(14);
  mapData = getWord(4);
  clt = getByte(7) - 40;
  afrConv = getByte(10);
  iat = getByte(6) - 40;
  tps = getByte(24) / 2;
  bat = getByte(9);
  adv = (int8_t)getByte(23);
  syncStatus = getBit(31, 7);
  ase = getBit(2, 2);
  wue = getBit(2, 3);
  rev = getBit(31, 2);
  launch = getBit(31, 0);
  airCon = getByte(122) / 10;
  fan = getBit(106, 3);

  drawData();
}
void drawDataBox(int x, int y, const char* label, const char* value, uint16_t labelColor, const char* valueToCompare) {
  const int BOX_WIDTH = 110;  // Reduced width to fit screen
  const int BOX_HEIGHT = 80; // Adjusted height

  // Check if value is different from previous value
  if (strcmp(valueToCompare, value) != 0) {
    const int LABEL_HEIGHT = BOX_HEIGHT / 2;
    if (firstLoop) {
      display.loadFont(AA_FONT_SMALL); 
      drawCenteredText(x, y, BOX_WIDTH, LABEL_HEIGHT, label, 2, labelColor);
    }
    display.loadFont(AA_FONT_LARGE); 
    drawCenteredText(x, y + LABEL_HEIGHT, BOX_WIDTH, LABEL_HEIGHT, value, 3, labelColor);
  }
}

void drawData() {
  if (firstLoop) {
    display.setTextSize(2);
    display.setTextColor(TFT_WHITE, TFT_BLACK);
    display.drawString("MAZDUINO_gank", 150, 5);
  }

  if (millis() - lazyUpdateTime > 1000) {
    const char* labelsLazy[] = {"IAT", "Coolant", "Voltage", "FPS"};
    int valuesLazy[] = {iat, clt, static_cast<int>(bat), refreshRate};
    int lastValuesLazy[] = {lastIat, lastClt, static_cast<int>(lastBat), lastRefreshRate};
    int positionsLazy[][2] = {{5, 10}, {5, 100}, {360, 100}, {240, 190}};
    uint16_t colorsLazy[] = {TFT_WHITE, (clt > 135) ? TFT_RED : TFT_WHITE, 
                             (bat < 11.5 || bat > 14.5) ? TFT_ORANGE : TFT_GREEN, TFT_WHITE};

    for (int i = 0; i < 4; i++) {
      char valueBuffer[22];
      formatValue(valueBuffer, valuesLazy[i], ( i == 2) ? 1 : 0);
      drawDataBox(positionsLazy[i][0], positionsLazy[i][1], labelsLazy[i], valueBuffer, colorsLazy[i], String(lastValuesLazy[i]).c_str());
      lastValuesLazy[i] = valuesLazy[i];
    }
    lazyUpdateTime = millis();
    firstLoop = false;
  }

  const char* labels[] = {"AFR", "TPS", "ADV", "MAP"};
  int values[] = {static_cast<int>(afrConv), tps, adv, mapData};
  int lastValues[] = {static_cast<int>(lastAfrConv), lastTps, lastAdv, lastMapData};
  int positions[][2] = {{5, 190}, {360, 190}, {120, 190}, {360, 10}};
  uint16_t colors[] = {(afrConv < 13) ? TFT_ORANGE : (afrConv > 14.8) ? TFT_RED : TFT_GREEN, TFT_WHITE, TFT_RED, TFT_WHITE};

  for (int i = 0; i < 4; i++) {
    char valueBuffer[22];
    formatValue(valueBuffer, values[i], (i == 0) ? 1 : 0);
    drawDataBox(positions[i][0], positions[i][1], labels[i], valueBuffer, colors[i], String(lastValues[i]).c_str());
    lastValues[i] = values[i];
  }

  if (lastRpm != rpm) {
    drawRPMBarBlocks(rpm);
    display.fillRect(185, 138, 130, 40, TFT_BLACK);
    if (firstLoop) {
      display.loadFont(AA_FONT_SMALL);
      display.setTextSize(2);
      display.setTextColor(TFT_WHITE, TFT_BLACK);
      display.drawString("RPM", 190, 120);
    }
    display.loadFont(AA_FONT_LARGE);
    display.setTextSize(4);
    char valueBuffer[22];
    sprintf(valueBuffer, "%u", rpm);
    int textWidth = display.textWidth(valueBuffer);
    int xPos = 185 + 130 - textWidth;
    display.drawString(valueBuffer, xPos, 140);
    lastRpm = rpm;
  }

  // Center buttons
  display.loadFont(AA_FONT_SMALL);
  const char* buttonLabels[] = {"SYNC", "FAN", "ASE", "WUE", "REV", "LCH", "AC"};
  bool buttonStates[] = {syncStatus, fan, ase, wue, rev, launch, airCon};
  for (int i = 0; i < 7; i++) {
    drawSmallButton(10 + 70 * i, 280, buttonLabels[i], buttonStates[i]);
  }
}