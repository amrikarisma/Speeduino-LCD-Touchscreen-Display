#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_FT6206.h>
#include <Fonts/FreeMonoBold18pt7b.h>

// Pin definition for ILI9341
#define TFT_CS     10
#define TFT_DC     9
#define TFT_RST    8 // You can also connect this to the Arduino RESET pin

Adafruit_ILI9341 display = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
Adafruit_FT6206 touchController;

int16_t PENRADIUS = 1;
uint16_t oldcolor, currentcolor;

// Color definitions
#define BLACK   0x0000
#define GRAY    0x8410
#define WHITE   0xFFFF
#define RED     0xF800
#define ORANGE  0xFA60
#define YELLOW  0xFFE0 
#define LIME    0x07FF
#define GREEN   0x07E0
#define CYAN    0x07FF
#define AQUA    0x04FF
#define BLUE    0x001F
#define MAGENTA 0xF81F
#define PINK    0xF8FF

static uint32_t oldtime = millis();
uint8_t speedyResponse[100]; //The data buffer for the Serial data. This is longer than needed, just in case
uint8_t byteNumber[2];  // pointer to which uint8_t number we are reading currently
uint8_t readiat; // clt doesn't need to be updated very often
int iat;   // to store coolant temp
uint8_t readclt; // clt doesn't need to be updated very often
int clt;   // to store coolant temp
int tps;
int flex;
int bat;
int adv;

unsigned int rpm;  //rpm and PW from speeduino
float afr;
float mapData;
int8_t psi;
float afrConv;
float batRead;
uint8_t cmdAdata[50] ; 

const int16_t CHAR_WIDTH = 6; // Lebar karakter default
const int16_t CHAR_HEIGHT = 8; // Tinggi karakter default


void setup () 
{
  Serial.begin(115200);
  
  // Initialize display
  Serial.println("Initializing display...");
  display.begin();
  Serial.println("Display initialized");

  display.setRotation(3); // Set display orientation
  display.fillScreen(ILI9341_BLACK);
  
  // Initialize touch controller
  if (!touchController.begin()) {
    Serial.println("Couldn't start FT6206 touchscreen controller");
    while (1);
  }

  delay(500);
}

uint32_t old_ts;
#define BYTES_TO_READ 74
#define SERIAL_TIMEOUT 300
float rps;
boolean sent = false;
boolean received = true; // change to true for test
uint32_t sendTimestamp;

void loop() {
  simulateData();
  requestData();
      drawData();

  if (received) {
    // displayData();
    // drawData();
    received = false;
  }
  
  handleTouch(); // Handle touch events
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

void displayData() {
  Serial.print("RPM-"); Serial.print(rpm); Serial.print("\t");
  Serial.print("CLT-"); Serial.print(clt); Serial.print("\t");
  Serial.print("MAP-"); Serial.print(psi); Serial.print("\t");
  Serial.print("AFR-"); Serial.print(afrConv); Serial.println("\t");
  Serial.print("IAT-"); Serial.print(iat); Serial.print("\t");
  Serial.print("TPS-"); Serial.print(tps); Serial.print("\t");
  Serial.print("BATT.V-"); Serial.print(bat); Serial.print("\t");
  Serial.print("FLEX %-"); Serial.print(flex); Serial.print("\t");
  Serial.print("ADVANCE°-"); Serial.print(adv); Serial.print("\t");
}

void drawData() { // Setup the mock area for drawing this info on the LCD
  
  // Judul
  display.setTextSize(1);
  display.setTextColor(AQUA);
  display.setCursor(10, 5); // Posisi judul di kiri atas
  display.print("Mazduino");
 
  // Bar TPS
  display.setTextSize(1);
  display.setTextColor(ORANGE);
  display.setCursor(290, 180);    
  display.print("TPS");
  display.fillRect(310, 150, 40, 50, BLACK);
  display.fillRect(310, 150 + (40 - tps), 50, tps,RED);

  // AFR
  display.setTextSize(2);
  display.setTextColor(ORANGE, BLACK);
  display.setCursor(10, 200); // Posisi label AFR
  display.print("AFR-");
  display.setTextSize(3);
  display.setCursor(70, 195); // Posisi nilai AFR
  display.print(afrConv, 1);

  // BOOST
  display.setTextSize(2);
  display.setTextColor(BLUE, BLACK);
  display.setCursor(150, 200); // Posisi label BOOST
  display.print("MAP-");
  display.setTextSize(3);
  display.setCursor(200, 195); // Posisi nilai BOOST
  display.print(psi * 0.0689);

  // CLT
  display.setTextSize(2);
  display.setTextColor(WHITE, BLACK);
  display.setCursor(10, 30); // Posisi label CLT
  display.print("CLT-");
  display.setCursor(50, 30); // Posisi nilai CLT
  display.print(clt - 40);

  // IAT
  display.setCursor(90, 30); // Posisi label IAT
  display.print("IAT-");
  display.setCursor(130, 30); // Posisi nilai IAT
  display.print(iat - 40);

  // TPS
  display.setCursor(170, 30); // Posisi label TPS
  display.print("TPS-");
  display.setCursor(210, 30); // Posisi nilai TPS
  display.print(tps);

  // RPM
  display.setTextSize(2);
  display.setCursor(10, 80); // Posisi label RPM
  display.print("RPM-");
  display.setTextSize(5);
  display.setCursor(35, 100); // Posisi nilai RPM
  display.print(rpm);

  // Battery Voltage
  display.setTextSize(2);
  display.setTextColor(RED, BLACK);
  display.setCursor(160, 60); // Posisi label BATT.V
  display.print("BATT.V-");
  display.setTextSize(3);
  display.setCursor(240, 55); // Posisi nilai BATT.V
  display.print(batRead,1);

  // Flex
  display.setTextSize(2);
  display.setTextColor(GREEN, BLACK);
  display.setCursor(160, 90); // Posisi label FLEX
  display.print("VE %-");
  display.setTextSize(3);
  display.setCursor(240, 85); // Posisi nilai FLEX
  display.print(flex);

  // Advance
  display.setTextSize(2);
  display.setCursor(160, 120); // Posisi label ADVANCE
  display.print("ADV-"); 
  display.setTextSize(3);
  display.setCursor(240, 115); // Posisi nilai ADVANCE
  display.print(adv);
  
  delay(1); 
}

void handleTouch() {
  if (touchController.touched()) {
    TS_Point point = touchController.getPoint();
    
    // Map raw coordinates to screen coordinates
    int x = map(point.x, 0, 240, 0, display.width());
    int y = map(point.y, 0, 320, 0, display.height());
    
    // Adjust for screen rotation if necessary
    if (display.getRotation() == 1) {
      int temp = x;
      x = display.width() - y;
      y = temp;
    }

    // Example: Draw a circle where touched
    display.fillCircle(x, y, PENRADIUS, WHITE);
  }
}

void processData() {  // necessary conversion for the data before sending to screen
 
  rpm = ((speedyResponse [15] << 8) | (speedyResponse [14])); // rpm low & high (Int) TBD: probaply no need to split high and low bytes etc. this could be all simpler
  afr = speedyResponse[10];
  mapData = ((speedyResponse [5] << 8) | (speedyResponse [4]));
  psi = (mapData / 6.895);
  clt = speedyResponse[7];
  afrConv = afr/10;
  iat = speedyResponse[6];
  tps = speedyResponse[24];
  bat = speedyResponse[9];
  batRead = bat/10;
  adv = speedyResponse[23];
  flex = speedyResponse[34];
  
}

// Fungsi untuk mensimulasikan data
void simulateData() {
  // Mengubah TPS, RPM, AFR, MAP, dll secara acak untuk simulasi
  tps = random(0, 100);           // TPS (0-100)
  rpm = random(1000, 8000);       // RPM (1000-8000)
  afrConv = random(12, 18);       // AFR (misalnya antara 12 hingga 18)
  psi = random(9000, 13000);            // BOOST (0-30 PSI)
  batRead = random(11.0, 15.0);    // Baterai (misalnya 1100 hingga 1500 mV)
  flex = random(50, 100);         // Flex Fuel (50-100%)
  adv = random(10, 40);           // Advance timing (10-40)

}