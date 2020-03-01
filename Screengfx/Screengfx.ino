// In the beginning we create the initial imports
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <SPI.h> // Communication with SPI devices 

// Pins required for Communication with Display
#define TFT_CS 10
#define TFT_RST 8
#define TFT_DC 9

// Tetrominos definition.
const int shape_color[SHAPE_NUM] = {CYAN,BLUE,ORANGE,GREEN,RED,YELLOW,MAGENTA};
const int shape_form[SHAPE_NUM][SHAPE_BLOCKS] = {
  {0,1,2,3}, // I
  {0,1,2,6}, // J
  {0,1,2,4}, // L
  {1,2,4,5}, // S
  {0,1,5,6}, // Z
  {1,2,5,6}, // O
  {0,1,2,5}  // T
};

// define the display class as tft
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);



void setup() {
  // put your setup code here, to run once:
  SPI.begin();
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(2);
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextSize(2);
  tft.setCursor(0,0);
  tft.setTextColor(ST77XX_WHITE);
  tft.println("Noisee");
  tft.drawPixel(20, 20, ST77XX_GREEN);
}

void loop() {
  // put your main code here, to run repeatedly:

}
