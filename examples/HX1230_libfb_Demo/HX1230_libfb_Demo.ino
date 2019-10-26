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

char buf[20];

void setup() 
{
  Serial.begin(9600);
  pinMode(LCD_BL, OUTPUT);
  //analogWrite(LCD_BL,30);
  digitalWrite(LCD_BL, HIGH);
  SPI.begin();
  lcd.init();
}

float t=0;

void animSinLines()
{
  lcd.cls();
  float x;
  for(int i=0;i<SCR_HT;i++) {
    x=15.0*(2.0+sin(t/10.0+i/18.0)*sin(i/9.0));
    lcd.drawLineHfastD(SCR_WD/2-x,SCR_WD/2+x,i,1);
    x=8.0*(2.0+sin(t/8.0+i/15.0)*sin(i/5.0));
    lcd.drawLineHfast(SCR_WD/2-x,SCR_WD/2+x,i,1);
  }
  t+=0.7;
  lcd.display();
}

char *banner="HX1230 LCD Lib";

void animTextSin()
{
  lcd.cls();
  lcd.setFont(c64enh);
  char *txt=banner;
  int x=4,i=0;
  float y;
  while(*txt) {
    y=20+10.0*(1.0+sin(t/10.0+i/6.0)*sin(t/18.0+i/9.0));
    x+=lcd.printChar(x,(int)y,*txt++);
    i++;
  }
  t+=0.7;
  lcd.display();
}

void animTextCircle()
{
  lcd.cls();
  lcd.setFont(c64enh);
  char *txt=banner;
  int x=4;
  float y,y2,xr;
  float r=10.0+6.0*(1.0+sin(t/28.0));
  float xo=25.0*(0.0+sin(t/20.0));
  float yt=24;
  while(*txt) {
    xr=2+x-SCR_WD/2-xo;
    y2=(r+3)*(r+3)-(xr)*(xr);
    y=yt;
    if(y2>0) y=SCR_HT/2-sqrt(y2)-5;
    if(y<0) y=0;
    if(y>yt) y=yt;
    x+=lcd.printChar(x,(int)y,*txt++);
  }
  
  lcd.fillCircleD(SCR_WD/2+xo,SCR_HT/2,r,1);
  //lcd.drawCircle(SCR_WD/2+xo,SCR_HT/2,r+1,1);
  t+=0.4;
  lcd.display();
}

void rects()
{
  int x,y;
  x = random(SCR_WD);
  y = random(40);
  lcd.setDither(random(12));
  lcd.fillRect(x,y,30,30,0); 
  lcd.fillRectD(x,y,30,30,1); 
  lcd.drawRect(x,y,30,30,1);
  lcd.display();
//delay(100);
}

void circles()
{
  int x,y;
  x = 15+random(SCR_WD-30);
  y = 15+random(SCR_HT-30);
  lcd.setDither(random(12));
  lcd.fillCircle(x,y,15,0);
  lcd.fillCircleD(x,y,15,1);
  lcd.drawCircle(x,y,16,1);
  lcd.display();
//delay(100);
}

int x=40,y=0;
int dx=6,dy=5;
int x2=80,y2=40;
int dx2=9,dy2=8;

void animRect()
{
  x+=dx;
  y+=dy;
  if(x>SCR_WD/2 || x<0) { dx=-dx; x+=dx; }
  if(y>SCR_HT/2 || y<0) { dy=-dy; y+=dy; }
  lcd.fillRect(x,y,SCR_WD/2,SCR_HT/2,2);
  lcd.display();
  lcd.fillRect(x,y,SCR_WD/2,SCR_HT/2,2);
  delay(40);
}

#define MAX_LINES 10
byte lx0[MAX_LINES];
byte lx1[MAX_LINES];
byte ly0[MAX_LINES];
byte ly1[MAX_LINES];
byte curLine=0;

void animLines()
{
  x+=dx;
  y+=dy;
  x2+=dx2;
  y2+=dy2;
  if(x>SCR_WD-1) { dx=-dx; x=SCR_WD-1; }
  if(x<1) { dx=-dx; x=0; }
  if(y>SCR_HT-1) { dy=-dy; y=SCR_HT-1; }
  if(y<1) { dy=-dy; y=0; }
  if(x2>SCR_WD-1) { dx2=-dx2; x2=SCR_WD-1; }
  if(x2<1) { dx2=-dx2; x2=0; }
  if(y2>SCR_HT-1) { dy2=-dy2; y2=SCR_HT-1; }
  if(y2<1) { dy2=-dy2; y2=0; }
  lx0[curLine]=x;
  lx1[curLine]=x2;
  ly0[curLine]=y;
  ly1[curLine]=y2;
  if(++curLine>=MAX_LINES) curLine=0;
  lcd.cls();
  for(int i=0;i<MAX_LINES;i++) {
    lcd.drawLine(lx0[i],ly0[i],lx1[i],ly1[i],1);
    lcd.drawLine(SCR_WD-1-lx0[i],ly0[i],SCR_WD-1-lx1[i],ly1[i],1);
    lcd.drawLine(lx0[i],SCR_HT-1-ly0[i],lx1[i],SCR_HT-1-ly1[i],1);
    lcd.drawLine(SCR_WD-1-lx0[i],SCR_HT-1-ly0[i],SCR_WD-1-lx1[i],SCR_HT-1-ly1[i],1);
  }
  lcd.display();
  delay(50);
}

unsigned long tm,demoTime=14000;

void loop() 
{
  tm = millis();
  lcd.cls();
  while(millis()-tm<demoTime) rects();

  tm = millis();
  lcd.cls();
  while(millis()-tm<demoTime) circles();

  lcd.setDither(8);
  tm = millis();
  while(millis()-tm<demoTime) animSinLines();

  tm = millis();
  while(millis()-tm<demoTime) animTextSin();

  tm = millis();
  while(millis()-tm<demoTime) animTextCircle();

  tm = millis();
  while(millis()-tm<demoTime) animLines();

  lcd.setFont(c64enh);
  lcd.fillRect(0,27,128,9,0);
  lcd.printStr(ALIGN_CENTER, 28, banner);
  x=40; y=0;
  dx=2; dy=1;
  tm = millis();
  while(millis()-tm<demoTime) animRect();
}

