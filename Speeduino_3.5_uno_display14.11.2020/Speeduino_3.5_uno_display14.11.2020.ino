#include "Arduino.h"
#include <MCUFRIEND_kbv.h>
MCUFRIEND_kbv display;       // hard-wired for UNO / MEGA shields anyway.
#include <WiFi.h>
#include <WiFiClient.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <esp_bt.h> 
#include <esp_wifi.h>

#include "colors.h"
#include "text_utils.h"
#include "drawing_utils.h"

#define UART_BAUD 115200
#define packTimeout 1
#define bufferSize 8192

const char *ssid = "MAZDUINO_ECU";
const char *pw = "123456789";
IPAddress ip(192, 168, 22, 33);
IPAddress netmask(255, 255, 255, 0);
const int port = 2000;

WiFiServer server(port);
WiFiClient client;
BLEServer *pServer = NULL;
BLECharacteristic *pTxCharacteristic = NULL;
BLECharacteristic *pRxCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint8_t buf1[bufferSize];
uint16_t i1 = 0;
uint8_t buf2[bufferSize];
uint16_t i2 = 0;
bool wifiConnected = false; 

#define SERVICE_UUID "6e400001-b5a3-f393-e0a9-e50e24dcca9e"
#define CHARACTERISTIC_UUID_TX "6e400002-b5a3-f393-e0a9-e50e24dcca9e"
#define CHARACTERISTIC_UUID_RX "6e400003-b5a3-f393-e0a9-e50e24dcca9e"

uint16_t ID;

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
bool syncStatus;
bool fan;
bool ase;
bool wue;
bool rev;
bool launch;
bool airCon; 

uint8_t cmdAdata[50]; 


class MyServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        deviceConnected = true;
    }

    void onDisconnect(BLEServer* pServer) {
        deviceConnected = false;
    }
};

class MyCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
        String rxValue = pCharacteristic->getValue();
        if (rxValue.length() > 0) {
            Serial1.write((uint8_t*)rxValue.c_str(), rxValue.length());
        }
    }
};

void setup () 
{
  display.reset();
  ID = display.readID();
  // 0x6814
  display.begin(ID);
  display.setRotation(1);
  display.fillScreen(BLACK);

  Serial.begin(UART_BAUD);
  Serial1.begin(UART_BAUD, SERIAL_8N1, 1, 0);  // UART1 ESP32-C3 SuperMini (RX sur GPIO1=>PA9, TX sur GPIO0=>PA10) dialogue STM32 Ok

  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(ip, ip, netmask);
  WiFi.softAP(ssid, pw);
  server.begin();

  esp_wifi_set_max_tx_power(78);

  BLEDevice::init("MAZDUINO_BLE");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pTxCharacteristic = pService->createCharacteristic(
                          CHARACTERISTIC_UUID_TX,
                          BLECharacteristic::PROPERTY_NOTIFY
                      );
  pTxCharacteristic->addDescriptor(new BLE2902());
  pRxCharacteristic = pService->createCharacteristic(
                          CHARACTERISTIC_UUID_RX,
                          BLECharacteristic::PROPERTY_WRITE
                      );
  pRxCharacteristic->setCallbacks(new MyCallbacks());
  pService->start();
  pServer->getAdvertising()->start();

  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_P9);
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_CONN_HDL0, ESP_PWR_LVL_P9);

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

  if (!client.connected()) {
      client = server.available();
      if (client) {
          wifiConnected = true; 
      } else {
          wifiConnected = false; 
      }
      return;
  }

  // Gérer les données reçues par WiFi
  if (client.available()) {
      while (client.available()) {
          buf1[i1] = (uint8_t)client.read();
          if (i1 < bufferSize - 1) i1++;
      }
      Serial1.write(buf1, i1);
      if (deviceConnected) {
          pTxCharacteristic->setValue(buf1, i1);
          pTxCharacteristic->notify();
      }
      i1 = 0;
  }

  // Gérer les données reçues par UART1
  if (Serial1.available()) {
      while (1) {
          if (Serial1.available()) {
              buf2[i2] = (char)Serial1.read();
              if (i2 < bufferSize - 1) i2++;
          } else {
              delay(packTimeout);
              if (!Serial1.available()) {
                  break;
              }
          }
      }
      client.write((char*)buf2, i2);
      if (deviceConnected) {
          pTxCharacteristic->setValue(buf2, i2);
          pTxCharacteristic->notify();
      }
      i2 = 0;
  }

  // Gérer la connexion BLE
  if (!deviceConnected && oldDeviceConnected) {
      delay(1); 
      pServer->startAdvertising(); 
      oldDeviceConnected = deviceConnected;
  }
  if (deviceConnected && !oldDeviceConnected) {
      oldDeviceConnected = deviceConnected;
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
  drawSmallButton(10, 280, "SYNC", syncStatus);
  drawSmallButton(80, 280, "FAN", fan);
  drawSmallButton(150, 280, "ASE", ase);
  drawSmallButton(220, 280, "WUE", wue);
  drawSmallButton(290, 280, "REV", rev);
  drawSmallButton(360, 280, "LAUNCH", launch);
  drawSmallButton(430, 280, "AC", airCon);

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
  syncStatus =   bitRead(speedyResponse[31], 7);
  ase = bitRead(speedyResponse[2], 2);
  wue = bitRead(speedyResponse[2], 3);
  rev = bitRead(speedyResponse[31], 2);
  launch = bitRead(speedyResponse[31], 0);
  airCon = speedyResponse[122];
  fan = bitRead(speedyResponse[106], 3);

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