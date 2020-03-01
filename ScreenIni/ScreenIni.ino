// In the beginning we create the initial imports
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <SPI.h> // Communication with SPI devices 

// Pins required for Communication with Display
#define TFT_CS 10
#define TFT_RST 8
#define TFT_DC 9

// define the display class as tft
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

void setup() {
  Serial.begin(9600);

  //Initialize Screen and create black background
  tft.initR(INITR_BLACKTAB);
  tft.fillScreen(ST7735_BLACK);

  // Does not write over the limits of the screen
  tft.setTextWrap(false);

  //print to display
  tft.println("Hello");
}



void loop() {
  // put your main code here, to run repeatedly:
  
}
