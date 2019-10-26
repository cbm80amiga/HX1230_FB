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
#include "tiny3x7sq_font.h"

char buf[20];

void setup() 
{
  Serial.begin(9600);
  pinMode(LCD_BL, OUTPUT);
  digitalWrite(LCD_BL, HIGH);
  lcd.init();
  lcd.cls();
  int dx = SCR_WD/16;
  for(int j=1;j<17;j++) { lcd.setDither(j); lcd.fillRectD((j-1)*dx,0,dx,SCR_HT,1); }
  lcd.display();
  delay(5000);
}

void sample(int s)
{
  lcd.cls();
  lcd.setDither(s);
  lcd.setFont(c64enh);
  snprintf(buf,20,"Pattern: %02d",s);
  lcd.printStr(ALIGN_CENTER, 2, buf);
  lcd.fillRectD(6,13,44,32,1);
  lcd.fillCircleD(96-24,28,16,1);
  int dx = SCR_WD/16;
  for(int j=1;j<17;j++) {
    lcd.setFont(Tiny3x7SqPL);
    buf[0]=(j>9) ? j-10+'A' : j+'0';
    buf[1]=0;
    lcd.printStr(2+(j-1)*dx, SCR_HT-8*2, buf);
    lcd.setDither(j);
    lcd.fillRectD((j-1)*dx,SCR_HT-8,dx,8,1);
  }
  lcd.display();
  delay(500);
}

void loop() 
{
  for(int i=0;i<=16;i++) sample(i);
  for(int i=16;i>0;i--) sample(i);
}

