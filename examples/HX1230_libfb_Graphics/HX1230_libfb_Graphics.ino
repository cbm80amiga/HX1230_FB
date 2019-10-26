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

void setup() 
{
  Serial.begin(9600);
  pinMode(LCD_BL, OUTPUT);
  digitalWrite(LCD_BL, HIGH);
  lcd.init();
  lcd.cls();
  lcd.drawRectD(0,0,SCR_WD,SCR_HT,1);

  lcd.drawRect(8,7,20,20,1);
  lcd.fillRect(8+30,7,20,20,1);
  lcd.fillRectD(8+60,7,20,20,1);

  lcd.drawCircle(5+12,48,12,1);
  lcd.fillCircle(5+30+12,48,12,1);
  lcd.fillCircleD(5+60+12,48,12,1);

  lcd.display();
}

void loop() 
{
}

