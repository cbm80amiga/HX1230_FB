// HX1230 96x68 LCD FB library example
// Gauges
// (c) 2019 Pawel A. Hernik
// YouTube video: https://youtu.be/x94y-qH2RBs

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

#include "bold13x20digtop_font.h"
#include "small4x6_font.h"

int cx,cy;
int sx,sy;
int xs0,ys0,xe0,ye0;
int xs1,ys1,xe1,ye1;
int i;
char buf[20];

// ------------------------------------------------
#define MAXSIN 255
const uint8_t sinTab[91] PROGMEM = {
0,4,8,13,17,22,26,31,35,39,44,48,53,57,61,65,70,74,78,83,87,91,95,99,103,107,111,115,119,123,
127,131,135,138,142,146,149,153,156,160,163,167,170,173,177,180,183,186,189,192,195,198,200,203,206,208,211,213,216,218,
220,223,225,227,229,231,232,234,236,238,239,241,242,243,245,246,247,248,249,250,251,251,252,253,253,254,254,254,254,254,
255
};

int fastSin(int i)
{
  if(i<0) i+=360;
  if(i>=360) i-=360;
  if(i<90)  return(pgm_read_byte(&sinTab[i])); else
  if(i<180) return(pgm_read_byte(&sinTab[180-i])); else
  if(i<270) return(-pgm_read_byte(&sinTab[i-180])); else
            return(-pgm_read_byte(&sinTab[360-i]));
}

int fastCos(int i)
{
  return fastSin(i+90);
}

// ------------------------------------------------

void drawGauge1(int level)
{
  lcd.cls();
  cx=96/2;
  cy=68-26;
  int rx0=35, ry0=30;
  int rx1=45, ry1=40;
  int mina=-40;
  int maxa=180+40;
  for(int i=mina; i<maxa; i+=20) {
    sx = fastCos(i-180);
    sy = fastSin(i-180);
    xs0 = cx+sx*rx0/MAXSIN;
    ys0 = cy+sy*ry0/MAXSIN;
    xe0 = cx+sx*rx1/MAXSIN;
    ye0 = cy+sy*ry1/MAXSIN;
    sx = fastCos(i-180+15);
    sy = fastSin(i-180+15);
    xs1 = cx+sx*rx0/MAXSIN;
    ys1 = cy+sy*ry0/MAXSIN;
    xe1 = cx+sx*rx1/MAXSIN;
    ye1 = cy+sy*ry1/MAXSIN;
    if(100*(i-mina)/(maxa-mina)<level) {
      lcd.fillTriangle(xs0,ys0,xe0,ye0,xe1,ye1, 1);
      lcd.fillTriangle(xs1,ys1,xe1,ye1,xs0,ys0, 1);
    } else {
      lcd.fillTriangleD(xs0,ys0,xe0,ye0,xe1,ye1, 1);
      lcd.fillTriangleD(xs1,ys1,xe1,ye1,xs0,ys0, 1);
    }
  }
  lcd.setFont(Bold13x20);
  snprintf(buf,10,"%d",level);
  lcd.printStr(ALIGN_CENTER,SCR_HT/2-2,buf);
  lcd.display();
}

void drawGauge2(int level)
{
  lcd.cls();
  cx=96/2;
  cy=62;
  int rx0=58;
  int rx1=rx0-3;
  int rx2=rx1-4;
  int mina=40;
  int maxa=180-40;
  int lastx,lasty;
  for(int i=mina; i<=maxa; i+=10) {
    sx = fastCos(i-180);
    sy = fastSin(i-180);
    xs0 = cx+sx*rx0/MAXSIN;
    ys0 = cy+sy*rx0/MAXSIN;
    xe0 = cx+sx*rx1/MAXSIN;
    ye0 = cy+sy*rx1/MAXSIN;
    lcd.drawLine(xs0,ys0,xe0,ye0,1);
    if(i>mina) lcd.drawLine(xs0,ys0,lastx,lasty,1);
    lastx=xs0;
    lasty=ys0;
    xs0 = cx+sx*rx2/MAXSIN;
    ys0 = cy+sy*rx2/MAXSIN;
    lcd.setFont(Small4x6PL);
    snprintf(buf,4,"%d",(i-40)/10);
    lcd.printStr(i==maxa?xs0-4:xs0-2,ys0-2,buf);
  }
  int a = map(level,0,100,mina,maxa);
  rx1=rx0-8;
  sx = fastCos(a-180);
  sy = fastSin(a-180);
  xs0 = cx+sx*rx1/MAXSIN;
  ys0 = cy+sy*rx1/MAXSIN;
  lcd.drawLine(xs0,ys0,cx,cy,1);
  lcd.fillCircle(cx,cy,2,1);

  rx1=rx0-26;
  sx = fastCos(a-180-8);
  sy = fastSin(a-180-8);
  xe0 = cx+sx*rx1/MAXSIN;
  ye0 = cy+sy*rx1/MAXSIN;
  sx = fastCos(a-180+8);
  sy = fastSin(a-180+8);
  xe1 = cx+sx*rx1/MAXSIN;
  ye1 = cy+sy*rx1/MAXSIN;
  lcd.fillTriangle(xs0,ys0,xe0,ye0,xe1,ye1,1);

  lcd.display();
}

void drawGauge3(int level)
{
  lcd.cls();
  cx=96/2;
  cy=68/2;
  int rx0=33;
  int rx1=rx0-4;
  int mina=0;
  int maxa=270;
  int lastx,lasty;
  for(int i=mina; i<=maxa; i+=3) {
    sx = fastCos(i-270);
    sy = fastSin(i-270);
    xs0 = cx+sx*rx0/MAXSIN;
    ys0 = cy+sy*rx0/MAXSIN;
    if(i>mina) lcd.drawLine(xs0,ys0,lastx,lasty,1);
    lastx=xs0;
    lasty=ys0;
  }
  for(int i=mina; i<=maxa; i+=9) {
    sx = fastCos(i-270);
    sy = fastSin(i-270);
    xs0 = cx+sx*rx0/MAXSIN;
    ys0 = cy+sy*rx0/MAXSIN;
    if(i%27==0) rx1=rx0-5; else rx1=rx0-3;
    xe0 = cx+sx*rx1/MAXSIN;
    ye0 = cy+sy*rx1/MAXSIN;
    lcd.drawLine(xs0,ys0,xe0,ye0,1);
  }

  int a = map(level,0,100,mina,maxa);
  rx1=rx0-7;
  sx = fastCos(a-270);
  sy = fastSin(a-270);
  xs0 = cx+sx*rx1/MAXSIN;
  ys0 = cy+sy*rx1/MAXSIN;
  lcd.drawLine(xs0,ys0,cx,cy,1);
  lcd.fillCircle(cx,cy,2,1);

  lcd.setFont(Bold13x20);
  snprintf(buf,10,"%d",level);
  lcd.printStr(ALIGN_RIGHT,SCR_HT-20,buf);
  lcd.display();
}

void drawGauge4(int level)
{
  lcd.cls();
  cx=96-1;
  cy=68-1;
  int rx0=cx-2, ry0=cy-2;
  int rx1=rx0-20, ry1=ry0-20;
  int rx2=rx0+2, ry2=ry0+2;
  int mina=0;
  int maxa=90;
  for(int i=mina; i<=maxa; i+=5) {
    sx = fastCos(i-180);
    sy = fastSin(i-180);
    xs0 = cx+sx*rx0/MAXSIN;
    ys0 = cy+sy*ry0/MAXSIN;
    xe0 = cx+sx*rx1/MAXSIN;
    ye0 = cy+sy*ry1/MAXSIN;
    //if(90*(i-mina)/(maxa-mina)<=level) lcd.drawLine(xs0,ys0,xe0,ye0, 1); else lcd.drawPixel(xs0,ys0, 1);
    if(i>mina) {
      if(i<=level) {
        lcd.fillTriangle(xs0,ys0,xe0,ye0,xe1,ye1, 1);
        lcd.fillTriangle(xs1,ys1,xe1,ye1,xs0,ys0, 1);
      } else {
        lcd.fillTriangleD(xs0,ys0,xe0,ye0,xe1,ye1, 1);
        lcd.fillTriangleD(xs1,ys1,xe1,ye1,xs0,ys0, 1);
      }
    }
    xs1 = xs0; ys1 = ys0;
    xe1 = xe0; ye1 = ye0;
    xe0 = cx+sx*rx2/MAXSIN;
    ye0 = cy+sy*ry2/MAXSIN;
    lcd.drawLine(xs0,ys0,xe0,ye0, 1);
  }
  lcd.setFont(Bold13x20);
  snprintf(buf,10,"%d",level);
  lcd.printStr(ALIGN_RIGHT,SCR_HT-20,buf);
  lcd.display();
}

void drawBar(int level)
{
  lcd.cls();
  int i=level*96/100;
  lcd.setDither(7+10*level/100);
  lcd.fillRectD(0,5,i,10,1);
  lcd.setDither(4);
  lcd.fillRectD(i,5,96-i,10,1);

  for(i=0;i<96;i+=8) {
    if(i<level*96/100) lcd.setDither(7+10*level/100); else lcd.setDither(2);
    lcd.fillRectD(i,68-15,7,10,1);
  }
  
  lcd.setFont(Bold13x20);
  snprintf(buf,10,"%d",level);
  lcd.printStr(ALIGN_CENTER,SCR_HT/2-10,buf);
  lcd.display();
}

int barLev[12]={0};
int barLevSet[12]={0};
int barDec[12]={0};

void drawEqBars()
{
  lcd.cls();
  for(int i=0;i<12;i++) {
    lcd.setDither(2+18*barLev[i]/68);
    lcd.fillRectD(i*8,67-barLev[i],7,barLev[i],1);
  }
  lcd.display();
  for(int i=0;i<12;i++) {
    if(!barDec[i]) {
      if(barLev[i]>barLevSet[i]) {
        barLev[i]-=4;
        if(barLev[i]<barLevSet[i]) { barLev[i]=barLevSet[i]; barDec[i]=1; } 
      } else {
        barLev[i]+=4;
        if(barLev[i]>barLevSet[i]) { barLev[i]=barLevSet[i]; barDec[i]=1; } 
      }
    } else {
      barLev[i]--;
      if(barLev[i]<0) barLev[i]=0;
    }
  }
  int r;
  r=random(12);
  barLevSet[r]=random(5,67); barDec[r]=0;
  if(r>0)  { barLevSet[r-1]=7*barLevSet[r]/10; barDec[r-1]=0; }
  if(r<11) { barLevSet[r+1]=7*barLevSet[r]/10; barDec[r+1]=0; }
}

unsigned long ms,tm=15000;

void setup(void) 
{
  Serial.begin(9600);
  pinMode(LCD_BL, OUTPUT);
  digitalWrite(LCD_BL, HIGH);
  lcd.init();
  lcd.cls();
  lcd.setFont(Small4x6PL);
  lcd.printStr(ALIGN_CENTER,SCR_HT/2-3,"GAUGES DEMO");
  lcd.display();
  delay(2000);
}

void loop() 
{
  ms=millis();
  while(millis()-ms<tm) {
    for(i=0;i<=100;i+=5) { drawGauge1(i); delay(30); }
    delay(250);
    for(i=100;i>=0;i-=5) { drawGauge1(i); delay(30); }
    delay(250);
  }
  
  ms=millis();
  while(millis()-ms<tm) {
    for(i=0;i<=100;i+=5) { drawBar(i); delay(60); }
    delay(250);
    for(i=100;i>=0;i-=5) { drawBar(i); delay(60); }
    delay(250);
  }

  ms=millis();
  while(millis()-ms<tm) {
    for(i=0;i<=100;i+=2) { drawGauge2(i); delay(40); }
    delay(450);
    for(i=100;i>=0;i-=2) { drawGauge2(i); delay(40); }
    delay(450);
  }

  ms=millis();
  while(millis()-ms<tm) {
    drawEqBars(); delay(20);
  }

  ms=millis();
  while(millis()-ms<tm) {
    for(i=0;i<=100;i+=2) { drawGauge3(i); delay(40); }
    delay(450);
    for(i=100;i>=0;i-=2) { drawGauge3(i); delay(40); }
    delay(450);
  }

  ms=millis();
  while(millis()-ms<tm) {
    for(i=0;i<=90;i+=5) { drawGauge4(i); delay(40); }
    delay(450);
    for(i=90;i>=0;i-=5) { drawGauge4(i); delay(40); }
    delay(450);
  }

}


