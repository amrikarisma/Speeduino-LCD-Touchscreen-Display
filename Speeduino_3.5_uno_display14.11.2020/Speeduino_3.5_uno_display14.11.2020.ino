    ///////////////////////////////////////////////////////////////////////////////////////////////////
   //                            SPEEDUINO ILI9486 TOUCH DISPLAY        v1.00   by TURBO MARIAN     //
  //                                 Get the display here:                                         //
 //https://www.amazon.co.uk/gp/product/B07N4KXP5X/ref=ppx_yo_dt_b_asin_title_o02_s00?ie=UTF8&th=1 //
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <MCUFRIEND_kbv.h>
MCUFRIEND_kbv display;       // hard-wired for UNO / MEGA shields anyway.
#include <TouchScreen.h>
#include  <Adafruit_GFX.h>
#include <Fonts/FreeMonoBold18pt7b.h>




const int XP=6,XM=A2,YP=A1,YM=7; //ID=0x9341
const int TS_LEFT = 907, TS_RT = 136, TS_TOP = 942, TS_BOT = 139;

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

TSPoint tp;

#define MINPRESSURE 200
#define MAXPRESSURE 1000

int16_t BOXSIZE;
int16_t PENRADIUS = 1;
uint16_t ID, oldcolor, currentcolor;
uint8_t Orientation = 1;    //PORTRAIT

#define DisplayLogo //Comment this out to disable display of the logo on bootup
 
//The following defined area was a bmp to hex conversion I did of my logo 
#ifdef DisplayLogo
#define imageWidth 128
#define imageHeight 32
const unsigned char bitmapData [] PROGMEM=
{
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xE0, 0x03, 0x87, 0x07, 0x00, 0x82, 0x08, 0x1C, 0x42,
0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x02, 0x40, 0x04,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x40, 0x00, 0x00, 0x00, 0x40, 0x02, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x07, 0x10, 0x40, 0x00, 0x00, 0x18, 0x40, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x90, 0x40, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x40, 0x10, 0x1E, 0x00, 0x47, 0x80, 0x80, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x40, 0x80, 0x00, 0x10, 0x01, 0x02, 0x40, 0x40, 0x40, 0x00, 0x00, 0x00, 0x00,
0x01, 0x80, 0x07, 0x07, 0x00, 0x10, 0x00, 0x00, 0x40, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
};
#endif

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
uint8_t readiat; // clt doesn't need to be updated very ofter so
int iat;   // to store coolant temp
uint8_t readclt; // clt doesn't need to be updated very ofter so
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
uint8_t cmdAdata[50] ; 

void setup () 
{

  
  display.reset();
  ID = display.readID();
  display.begin(ID);
  Serial.begin(115200);
  display.setRotation(1);
  display.fillScreen(BLACK);
  
  delay(500);

  }
    

uint32_t old_ts;

#define BYTES_TO_READ 74
#define SERIAL_TIMEOUT 300
float rps;
boolean sent = false;
boolean received = true;
uint32_t sendTimestamp;

void loop () {
  requestData();
  if(received) {
    // displayData();
    drawData();
    received = false;
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
  Serial.print("MAP-"); Serial.print(psi); Serial.print("\t");
  Serial.print("AFR-"); Serial.print(afrConv); Serial.println("\t");
  Serial.print("IAT-"); Serial.print(iat); Serial.print("\t");
  Serial.print("TPS-"); Serial.print(tps); Serial.print("\t");
  Serial.print("BATT.V-"); Serial.print(bat); Serial.print("\t");
  Serial.print("FLEX %-"); Serial.print(flex); Serial.print("\t");
  Serial.print("ADVANCE°-"); Serial.print(adv); Serial.print("\t");
}


 
void drawData() { //Setup the mock area for drawing this info on the OLED
  
  

  display.setTextSize(1);
  display.setTextColor(AQUA);
  display.setCursor(350,0);
  display.print("MAZDUINO_Gank");
 
  
  display.setTextSize(1);
  display.setTextColor(ORANGE);
  display.setCursor(440,310);
  display.print("TPS");
  display.fillRect(430, 260, 40, 40, BLACK);
  display.fillRect(430, 260 + (40 - tps), 40, tps,RED);
  
  
  
  display.setTextSize(2);            // Draw 2X-scale text
  display.setTextColor(ORANGE,BLACK);
  display.setCursor(5,280);
  display.print("AFR-");
  display.setTextSize(4);
  display.setCursor(80,275);
  display.print(afrConv, 1);
  
  display.setTextSize(2);
  display.setTextColor(BLUE,BLACK);
  display.setCursor(210,280);
  display.print("MAP-");
  display.setTextSize(4);
  display.setCursor(310,275);
  display.print(psi*0.0689);
  
  display.setTextSize(2);
  display.setCursor(0,0);
  display.print("CLT-");
  display.setTextSize(2);
  display.setCursor(40,0);
  display.print(clt-40);       // offset -40  C readout ( removed -40to get F readout)

  display.setTextSize(2);
  display.setCursor(80,0);
  display.print("IAT-");
  display.setTextSize(2);
  display.setCursor(120,0);
  display.print(iat-40);        // offset -40  C readout ( removed -40to get F readout)
  
  display.setTextSize(2);
  display.setCursor(160,0);
  display.print("TPS-");
  display.setTextSize(2);
  display.setCursor(200,0);
  display.print(tps);    
  
  display.setTextSize(2);
  display.setCursor(30,100);
  display.print("RPM-");
  display.setTextSize(5);
  display.setCursor(90,120);
  display.print(rpm);

  display.setTextSize(2);
  display.setTextColor(RED,BLACK);
  display.setCursor(300,60);
  display.print("BATT.V-");
  display.setTextSize(3);
  display.setCursor(390,55);
  display.print(bat/10);

  display.setTextSize(2);
  display.setTextColor(ORANGE,BLACK);
  display.setCursor(300,90);
  display.print("CLT-");
  display.setTextSize(3);
  display.setCursor(390,85);
  display.print(clt-40);       // offset -40  C readout ( removed -40to get F readout)

  display.setTextSize(2);
  display.setTextColor(GREEN,BLACK);
  display.setCursor(300,120);
  display.print("ADVANCE-"); 
  display.setTextSize(3);
  display.setCursor(405,115);
  display.print(adv);
  
  delay(40);  // this delay was placed in order for the screen to not populate at a speed that is inhumanly readable
  
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
  adv = speedyResponse[23];
  flex = speedyResponse[34];
  
}

void runSimulator() {
  rpm = random(700,6000);
}