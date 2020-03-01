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

// initialize integer
int number_on_screen;

void setup() {
  
  //Initialize Screen and create black background
  tft.initR(INITR_BLACKTAB);
  tft.fillScreen(ST7735_BLACK);

  // Does not write over the limits of the screen
  tft.setTextWrap(false);

  //print "Hello" to display in White in the beginning of the display
  tft.setCursor(0,0);
  tft.setTextColor(ST7735_WHITE);
  tft.setTextSize(3);
  tft.println("Hello");

  //print World to display in Red as of row 10 from the top and indented to column 30
  tft.setCursor(10,30);
  tft.setTextColor(ST7735_RED);
  tft.setTextSize(3);
  tft.println("World!");

  //draw rectangle in blue as box for the number_on_screen
  tft.drawRect(0, 60, 60, 30, ST7735_CYAN);  // Draw rectangle (x,y,width,height,color)
                                             // It draws from the location to down-right

  //draw a line in blue                                        
  tft.drawLine(0, 125, 127, 125, ST7735_CYAN);  // Draw line (x0,y0,x1,y1,color)

}



void loop() {
  //print the number in the box and update it
  
  Variable1++;  // Increase variable by 1
  
  if(Variable1 > 150)  // If Variable1 is greater than 150
  {
    Variable1 = 0;  // Set Variable1 to 0
  }

  char string[10];  // Create a character array of 10 characters

  dtostrf(Variable1, 3, 0, string);  // (<variable>,<amount of digits we are going to use>,<amount of decimal digits>,<string name>)

  tft.setCursor(13, 67);  // Set position (x,y)
  tft.setTextColor(ST7735_YELLOW, ST7735_BLACK);  // Set color of text. First is the color of text and after is color of background
  tft.setTextSize(2);  // Set text size. Goes from 0 (the smallest) to 20 (very big)
  tft.println(Variable1);  // Print a text or value
  
}
