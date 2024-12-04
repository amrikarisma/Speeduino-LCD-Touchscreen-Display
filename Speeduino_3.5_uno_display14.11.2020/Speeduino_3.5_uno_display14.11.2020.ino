#include "Arduino.h"
#include <MCUFRIEND_kbv.h>
MCUFRIEND_kbv display;       // hard-wired for UNO / MEGA shields anyway.
#include <TouchScreen.h>
#include  <Adafruit_GFX.h>
// #include <Fonts/FreeMonoBold18pt7b.h>

#include "colors.h"
#include "text_utils.h"
#include "drawing_utils.h"



const int XP=6,XM=A2,YP=A1,YM=7; //ID=0x9341
const int TS_LEFT = 907, TS_RT = 136, TS_TOP = 942, TS_BOT = 139;

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

TSPoint tp;

#define MINPRESSURE 200
#define MAXPRESSURE 1000

int16_t BOXSIZE;
int16_t PENRADIUS = 1;
uint16_t ID, oldcolor, currentcolor;

static uint32_t oldtime = millis();
uint8_t speedyResponse[100]; //The data buffer for the Serial data. This is longer than needed, just in case
uint8_t byteNumber[2];  // pointer to which uint8_t number we are reading currently
uint8_t readiat; // clt doesn't need to be updated very ofter so
int iat;   // to store coolant temp
uint8_t readclt; // clt doesn't need to be updated very ofter so
int clt;   // to store coolant temp
int tps;
float bat;
int adv;
unsigned int rpm;  //rpm and PW from speeduino
float afr;
int mapData;
int8_t psi;
float afrConv;
bool sync;
bool fan;
bool ase;
bool wue;
bool rev;
bool launch;
bool ac; 

uint8_t cmdAdata[50]; 

void setup () 
{
  display.reset();
  ID = display.readID();
  display.begin(ID);
  display.setRotation(1);
  display.fillScreen(BLACK);

  Serial.begin(115200);
  delay(500);

  }
    

#define BYTES_TO_READ 74
#define SERIAL_TIMEOUT 300
float rps;
boolean sent = false;
boolean received = true;
uint32_t sendTimestamp;

void loop () {
  requestData();
  runSimulator();
  if(received) {
    // displayData();
    drawRPMBarBlocks(rpm);
    drawData();
    received = true;
  }
}


void requestData() {
  if(sent && Serial.available()) {
    if(Serial.read() == 'A') {
      uint8_t bytesRead = Serial.readBytes(speedyResponse, BYTES_TO_READ);
      if(bytesRead != BYTES_TO_READ) {
        processData();
        for(uint8_t i = 0; i < bytesRead; i++) {
        }
        received = true;
        clearRX();
      } else {
        processData();
        received = true;
        rps = 1000.0/(millis() - sendTimestamp);
      }
      sent = false;
    } else Serial.read();
  } else if(!sent) {
    Serial.write('A');
    sent = true;
    sendTimestamp = millis();
  } else if(sent && millis() - sendTimestamp > SERIAL_TIMEOUT) {
    sent = false;
  }
}

void clearRX() {
  while(Serial.available()) Serial.read();
}
 
//display the needed values in serial monitor for debugging
void displayData() {
  Serial.print("RPM-"); Serial.print(rpm); Serial.print("\t");
  Serial.print("CLT-"); Serial.print(clt); Serial.print("\t");
  Serial.print("MAP-"); Serial.print(mapData); Serial.print("\t");
  Serial.print("AFR-"); Serial.print(afrConv); Serial.println("\t");
  Serial.print("IAT-"); Serial.print(iat); Serial.print("\t");
  Serial.print("TPS-"); Serial.print(tps); Serial.print("\t");
  Serial.print("BATT.V-"); Serial.print(bat); Serial.print("\t");
  Serial.print("ADVANCE°-"); Serial.print(adv); Serial.print("\t");
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

  char valueBuffer[10]; // Buffer for converting numbers to strings
  display.setTextSize(2);
  display.setCursor(150, 5);
  display.print("MAZDUINO_gank");

  // Left Column
  // IAT
  snprintf(valueBuffer, sizeof(valueBuffer), "%d", iat);
  drawDataBox(5, 10, "IAT", valueBuffer, WHITE);

  // Coolant
  snprintf(valueBuffer, sizeof(valueBuffer), "%d", clt - 40);
  if (clt > 135) {
    drawDataBox(5, 100, "Coolant", valueBuffer, RED);
  } else {
    drawDataBox(5, 100, "Coolant", valueBuffer, WHITE);
  }

  // AFR
  dtostrf(afrConv, 2, 2, valueBuffer);
  if (afrConv<13 ) {
    drawDataBox(5, 190, "AFR", valueBuffer, ORANGE);
  } else if(afrConv>14.8) {
    drawDataBox(5, 190, "AFR", valueBuffer, RED);
  } else {
    drawDataBox(5, 190, "AFR", valueBuffer, GREEN);
  }

  // Center Column
  // RPM
  display.setTextSize(2);
  display.setTextColor(WHITE, BLACK);
  display.setCursor(190, 120);
  display.print("RPM");
  display.setCursor(190, 140);
  display.setTextSize(4);
  snprintf(valueBuffer, sizeof(valueBuffer), "%d", rpm);
  display.print(valueBuffer);

  // Center buttons
  drawSmallButton(10, 280, "SYNC", sync);
  drawSmallButton(80, 280, "FAN", fan);
  drawSmallButton(150, 280, "ASE", ase);
  drawSmallButton(220, 280, "WUE", wue);
  drawSmallButton(290, 280, "REV", rev);
  drawSmallButton(360, 280, "LAUNCH", launch);
  drawSmallButton(430, 280, "AC", ac);

  // ADVANCE
  snprintf(valueBuffer, sizeof(valueBuffer), "%d", adv);
  drawDataBox(185, 190, "ADVANCE", valueBuffer, RED);

  // Right Column
  // MAP
  snprintf(valueBuffer, sizeof(valueBuffer), "%d", mapData);
  drawDataBox(360, 10, "MAP", valueBuffer, WHITE);

  // Voltage
  dtostrf(bat, 2, 2, valueBuffer);
  if (bat<11.5 | bat > 14.5) {
    drawDataBox(360, 100, "Voltage", valueBuffer, ORANGE);
  } else {
    drawDataBox(360, 100, "Voltage", valueBuffer, GREEN);
  }
  // TPS
  snprintf(valueBuffer, sizeof(valueBuffer), "%d%%", tps);
  drawDataBox(360, 190, "TPS", valueBuffer, WHITE);



  delay(40);
}
void processData() {  // necessary conversion for the data before sending to screen
 
  rpm = ((speedyResponse[15] << 8) | (speedyResponse[14])); // rpm low & high (Int) TBD: probaply no need to split high and low bytes etc. this could be all simpler
  afr = speedyResponse[10];
  mapData = ((speedyResponse[5] << 8) | (speedyResponse[4]));
  psi = (mapData / 6.895);
  clt = speedyResponse[7];
  afrConv = afr/10;
  iat = speedyResponse[6];
  tps = speedyResponse[24];
  bat = speedyResponse[9];
  adv = speedyResponse[23];
  sync =   bitRead(speedyResponse[31], 7);
  ase = bitRead(speedyResponse[2], 2);
  wue = bitRead(speedyResponse[2], 3);
  rev = bitRead(speedyResponse[31], 2);
  launch = bitRead(speedyResponse[31], 0);

}

void runSimulator() {
  rpm = random(0,8000);
  tps = random(0,100);
  iat = random(45,67);
  clt = random(120,140);
  mapData = random(0,100);
  afrConv = random(100,200)/10;
  bat = 14.2;
}