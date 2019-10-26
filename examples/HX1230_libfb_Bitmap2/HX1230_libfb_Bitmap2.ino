// HX1230 96x68 LCD FB library example
// (c) 2019 Pawel A. Hernik

/*
  HX1230 96x68 LCD connections (header on bottom, from left):
  #1 RST - D6 or any digital
  #2 CE  - D7 or any digital
  #3 N/C
  #4 DIN - D11/MOSI 
  #5 CLK - D13/SCK
  #6 VCC - 3V3
  #7 BL  - 3V3 or any digital
  #8 GND - GND
*/

#define LCD_RST 6
#define LCD_CS  7
#define LCD_BL  8

#include "HX1230_FB.h"
#include <SPI.h>
HX1230_FB lcd(LCD_RST, LCD_CS);

const uint8_t cat [] PROGMEM = {35,6*8,
0x00, 0x00, 0x00, 0x00, 0xF8, 0x0C, 0x06, 0x02, 0x0E, 0x30, 0x60, 0x20, 0x20, 0x40, 0x80, 0x00,
0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x78, 0x8E, 0x03, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x01, 0xF9, 0x0C, 0x87, 0xC0, 0x60, 0x30, 0x18, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
0x08, 0x10, 0x20, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0xFC, 0x00, 0x00, 0x00, 0x00,
0xF0, 0x10, 0x18, 0x08, 0x0C, 0x06, 0x03, 0xFC, 0x07, 0x00, 0x00, 0x80, 0xE0, 0x38, 0x08, 0x0C,
0x04, 0x04, 0x04, 0x04, 0x04, 0x08, 0x18, 0x0F, 0x00, 0x00, 0x00, 0xC0, 0x78, 0x0F, 0x01, 0x00,
0x00, 0x00, 0x00, 0x00, 0x07, 0x18, 0x20, 0xC0, 0x00, 0x00, 0x01, 0x0E, 0xF0, 0x00, 0x07, 0xF8,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x7E, 0x03, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x1C, 0xE0, 0xC0, 0x7D,
0x06, 0x00, 0xF3, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
0x0E, 0x10, 0x20, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x60,
0x20, 0x31, 0x18, 0x0C, 0x07, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 
};

const uint8_t dog [] PROGMEM = {47,6*8,
0x70, 0x90, 0x18, 0x08, 0x0C, 0x04, 0x06, 0x02, 0x04, 0x04, 0x06, 0x02, 0x02, 0x02, 0x02, 0x02,
0x04, 0x06, 0x02, 0x04, 0x04, 0x04, 0x08, 0x38, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x01, 0x02, 0x04, 0x08, 0x18, 0x60, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x60, 0xBC, 0x84, 0x88, 0x9E, 0x83, 0x00, 0x00, 0x80, 0x80, 0xC0, 0x40, 0x60, 0x20,
0x30, 0x90, 0x98, 0xC8, 0x48, 0x68, 0x28, 0x28, 0x28, 0x28, 0x28, 0x28, 0x38, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0xE0, 0x3C, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x06, 0x1A, 0xE3,
0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0xEF, 0x30, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xC0, 0x40, 0x40, 0x40, 0x80, 0x00, 0x00, 0x07,
0x18, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x07, 0xF8, 0x00, 0x00, 0x00, 0xFE, 0x03, 0x00, 0x01, 0x01, 0x02, 0x02, 0x02, 0xC2, 0x7E,
0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x3C, 0xC0, 0x00,
0x03, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0xE0, 0xB0,
0x98, 0x8F, 0x80, 0x80, 0xC0, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x0B, 0x49, 0xF8, 0x80,
0x80, 0x80, 0x80, 0x8F, 0xF1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1E, 0x23, 0x20, 0x20,
0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
};


void setup() 
{
  Serial.begin(9600);
  pinMode(LCD_BL, OUTPUT);
  digitalWrite(LCD_BL, HIGH);
  lcd.init();
  lcd.cls();
  lcd.drawBitmap(dog,ALIGN_LEFT,10);
  lcd.drawBitmap(cat,ALIGN_RIGHT,10);
  lcd.display();
}

void loop() 
{
}

