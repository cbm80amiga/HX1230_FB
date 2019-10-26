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

// from PropFonts library
#include "c64enh_font.h"

void setup() 
{
  Serial.begin(9600);
  pinMode(LCD_BL, OUTPUT);
  //analogWrite(LCD_BL,30);
  digitalWrite(LCD_BL, HIGH);

  SPI.begin();
  lcd.init();
  lcd.cls();
  lcd.setFont(c64enh);
  lcd.printStr(ALIGN_CENTER, 30, "Hello World!");
  lcd.drawRect(4,20,95-4*2,67-20*2,1);
  lcd.display();
}

void loop() 
{
}

