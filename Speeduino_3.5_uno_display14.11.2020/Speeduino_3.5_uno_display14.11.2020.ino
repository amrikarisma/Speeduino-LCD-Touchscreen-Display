#include "Arduino.h"
#include "SPI.h"
#include <TFT_eSPI.h>  // Library TFT_eSPI
TFT_eSPI display = TFT_eSPI(); // Create TFT instance

#include "Comms.h"
#include "text_utils.h"
#include "drawing_utils.h"

#define UART_BAUD 115200
#define packTimeout 1
#define bufferSize 8192

uint16_t ID;
bool staticDraw = false;

uint8_t readiat;
int iat;
uint8_t readclt;
int clt;
int tps;
float bat;
int adv;
unsigned int rpm = 0;
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

#define RXD 16
#define TXD 17

void setup() {
  display.init(); // Initialize display
  display.setRotation(1); // Set rotation
  display.fillScreen(TFT_BLACK); // Clear screen with black

  Serial.begin(UART_BAUD);
  Serial1.begin(UART_BAUD, SERIAL_8N1, RXD, TXD);
  delay(500);
}

boolean sent = false;
boolean received = true;
uint32_t sendTimestamp;
uint16_t refreshRate;

void loop() {
  static uint32_t lastUpdate = millis();
  if (millis() - lastUpdate > 100) {
    requestData(90);
    lastUpdate = millis();
  }

  // static uint32_t lastRefresh = millis();
  // uint32_t elapsed = millis() - lastRefresh;
  // if (elapsed > 0) {
  //     refreshRate = 1000 / elapsed;
  // } else {
  //     refreshRate = 0; 
  // }
  // lastRefresh = millis();

  // Data acquisition
  // rpm = getWord(14); 
  mapData = getWord(4);
  // clt = (int16_t)getByte(7) - 40;
  // afrConv = getByte(10);
  // iat = getByte(6) - 40;
  // tps = getByte(24) / 2; // TPS is calculated here
  // bat = getByte(9);
  // adv = (int8_t)getByte(23);
  // syncStatus = getBit(31, 7);
  // ase = getBit(2, 2);
  // wue = getBit(2, 3);
  // rev = getBit(31, 2);
  // launch = getBit(31, 0);
  // airCon = getByte(122) / 10;
  // fan = getBit(106, 3);
  debugData();
  drawData();
}
void debugData() {
  Serial.println(ESP.getFreeHeap());
}
// void drawDataBox(int x, int y, const char* label, const char* value, uint16_t labelColor) {
//   const int BOX_WIDTH = 110;
//   const int BOX_HEIGHT = 80;

//   // display.fillRoundRect(x, y, BOX_WIDTH, BOX_HEIGHT, 5, TFT_BLACK);
//   display.setTextColor(labelColor, TFT_BLACK);
//   display.setTextDatum(MC_DATUM);
//   display.setTextSize(2);
//   display.drawString(label, x + BOX_WIDTH / 2, y + BOX_HEIGHT / 4);
//   display.setTextSize(3);
//   display.drawString(value, x + BOX_WIDTH / 2, y + 3 * BOX_HEIGHT / 4);
// }

void drawData() {
  char valueBuffer[22];

  if (!staticDraw) {
    display.setTextSize(2);
    display.setTextColor(TFT_WHITE, TFT_BLACK);
    display.drawString("MAZDUINO_gank", 150, 5);
    staticDraw = true;
  }


  // // IAT
  // sprintf(valueBuffer, "%d", mapData);
  // drawDataBox(5, 10, "IAT", valueBuffer, TFT_WHITE);
  // display.setTextColor(TFT_WHITE, TFT_BLACK);
  // display.setTextSize(2);
  // display.drawString("2", 150, 150 );
  // // Coolant
  // sprintf(valueBuffer, "%d", clt);
  // drawDataBox(5, 100, "Coolant", valueBuffer, (clt > 135) ? TFT_RED : TFT_WHITE);

  // // AFR
  // sprintf(valueBuffer, "%.1f", afrConv);
  // drawDataBox(5, 190, "AFR", valueBuffer, (afrConv < 13) ? TFT_ORANGE : (afrConv > 14.8) ? TFT_RED : TFT_GREEN);

  // // RPM
  // if (lastRpm != rpm) {
  //   display.fillRect(185, 138, 130, 40, TFT_BLACK);
  //   display.setTextSize(2);
  //   display.setTextColor(TFT_WHITE, TFT_BLACK);
  //   display.drawString("RPM", 190, 120);
  //   display.setTextSize(4);
  //   sprintf(valueBuffer, "%u", rpm);
  //   display.drawString(valueBuffer, 190, 140);
  //   lastRpm = rpm;
  // }

  // // TPS (added display for TPS)
  // sprintf(valueBuffer, "%d", tps);
  // drawDataBox(360, 190, "TPS", valueBuffer, TFT_WHITE);

  // // MAP
  // sprintf(valueBuffer, "%d", mapData);
  // drawDataBox(360, 10, "MAP", valueBuffer, TFT_WHITE);

  // // Voltage
  // sprintf(valueBuffer, "%.1f", bat);
  // drawDataBox(360, 100, "Voltage", valueBuffer, (bat < 11.5 || bat > 14.5) ? TFT_ORANGE : TFT_GREEN);
  delay(1000);
}