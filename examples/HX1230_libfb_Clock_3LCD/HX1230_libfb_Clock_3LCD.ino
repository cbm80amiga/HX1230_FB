// HX1230 96x68 LCD FB library example
// Analog Clock without floating-point calculations, no slow sin, cos
// Digital Clock requires RREFont library
// (c) 2019 Pawel A. Hernik
// YouTube videos:
// https://youtu.be/nRJxjmrr09A
// https://youtu.be/uBId9KbtGuQ

/*
  HX1230 96x68 LCD connections (header at the bottom, from left):
  #1 RST - D6 shared by all 3 LCDs
  #2 CE/CS - D7,D8,D9 separate for each LCD
  #3 N/C
  #4 DIN - D11/MOSI shared by all 3 LCDs
  #5 CLK - D13/SCK shared by all 3 LCDs
  #6 VCC - 3V3
  #7 BL  - 3V3 or any digital
  #8 GND - GND
*/

#define LCD_RST 6
#define LCD_CS1 7
#define LCD_CS2 8
#define LCD_CS3 9
//#define LCD_BL  10

#include "HX1230_FB.h"
#include <SPI.h>
HX1230_FB lcd1(LCD_RST, LCD_CS1); // only 1 reset pin defined
HX1230_FB lcd2(255, LCD_CS2);
HX1230_FB lcd3(255, LCD_CS3);

#include "small4x6_font.h"

#include "RREFont.h"
#include "rre_arialdig47b.h"
#include "rre_bold13x20.h"
#include "rre_arialbldig54b.h"

RREFont font;

// needed for RREFont library initialization, define your fillRect
// here RRE renders fonts in common frambuffer so only lcd1 is used
void customRect1(int x, int y, int w, int h, int c) { return lcd1.fillRect(x, y, w, h, c); }
void customRectDither1(int x, int y, int w, int h, int c) { return lcd1.fillRectD(x, y, w, h, c); }


// analog clock configuration

int cx = SCR_WD/2, cy = SCR_HT/2;  // clock center
int rline0 = 34, rline1 = 30, rdot = 33, rnum = 25; // radius for hour lines, minute dots and numbers
int rhr = 16, rmin = 19, rsec = 26;                 // hour, minute, second hand length
int hrNum = 1;                                      // hour numbers - 0=none, 3-only 12,3,6,9, 1-all
int qdots = 0;                                      // quadrant dots

uint8_t txt2num(const char* p) 
{
  return 10*(*p-'0') + *(p+1)-'0';
}

int sx,sy,mx,my,hx,hy;
int sdeg,mdeg,hdeg;
int osx,osy,omx,omy,ohx,ohy;
int xs,ys,xe,ye;
unsigned long ms,startMillis;
unsigned long startTime=16*3600L+58*60+50;
uint8_t hh = txt2num(__TIME__+0);
uint8_t mm = txt2num(__TIME__+3);
uint8_t ss = txt2num(__TIME__+6);
uint8_t start = 1;
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
  while(i<0) i+=360;
  while(i>=360) i-=360;
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

void setup(void) 
{
  Serial.begin(9600);
  //pinMode(LCD_BL, OUTPUT);
  //digitalWrite(LCD_BL, HIGH);
  lcd1.init();
  lcd2.init();
  lcd3.init();
  lcd3.setFont(Small4x6PL);

  font.init(customRect1, SCR_WD, SCR_HT); // custom fillRect function and screen width and height values
  ms = startMillis = millis();
  startTime = hh*3600L+mm*60+ss + 10;  // 10 - delay between real time and upload
}

void loop() 
{
  if(ss>15 && ss<45) analogClock(); else digitalClock();
}

void tick()
{
  unsigned long tim = startTime+(millis()-startMillis)/1000;
  hh = (tim/3600L);
  while(hh>23) hh-=24;
  mm = (tim-hh*3600L)/60L;
  ss = tim-hh*3600L-mm*60L;
}

int hhOld=hh,mmOld=mm,ssOld=ss;

// hh,mm,ss on separate LCD
void digitalClock()
{
  if(millis()-ms<1000 && !start) return;
  ms = millis();
  start = 0;
  hhOld = hh;
  mmOld = mm;
  ssOld = ss;
  tick();
  font.setFont(&rre_ArialBlDig54b);  font.setDigitMinWd(46);
  font.setFillRectFun(customRectDither1);

  for(int i=1;i<=16;i++) {
    lcd1.cls();
    snprintf(buf,5,"%02d",hhOld);
    lcd1.setDither(-i);
    font.printStr(ALIGN_CENTER,(SCR_HT-font.getHeight())/2,buf);
    lcd1.setDither(i);
    snprintf(buf,5,"%02d",hh);
    font.printStr(ALIGN_CENTER,(SCR_HT-font.getHeight())/2,buf);
    lcd1.display();

    lcd2.cls();
    snprintf(buf,5,"%02d",mmOld);
    lcd1.setDither(-i);
    font.printStr(ALIGN_CENTER,(SCR_HT-font.getHeight())/2,buf);
    lcd1.setDither(i);
    snprintf(buf,5,"%02d",mm);
    font.printStr(ALIGN_CENTER,(SCR_HT-font.getHeight())/2,buf);
    lcd2.display();

    lcd3.cls();
    snprintf(buf,5,"%02d",ssOld);
    lcd1.setDither(-i);
    font.printStr(ALIGN_CENTER,(SCR_HT-font.getHeight())/2,buf);
    lcd1.setDither(i);
    snprintf(buf,5,"%02d",ss);
    font.printStr(ALIGN_CENTER,(SCR_HT-font.getHeight())/2,buf);
    lcd3.display();
    //delay(50);
  }
}

int outlineD(int x, int y, char *str, int dither)
{
  font.setColor(1);
  font.setFillRectFun(customRect1);
  font.printStr(x-1,y-1,str);
  font.printStr(x-0,y-1,str);
  font.printStr(x+1,y-1,str);
  font.printStr(x-1,y+1,str);
  font.printStr(x-0,y+1,str);
  font.printStr(x+1,y+1,str);
  font.printStr(x-1,y,str);
  font.printStr(x+1,y,str);
  font.setColor(0);
  font.setFillRectFun(customRectDither1);
  lcd1.setDither(dither);
  font.printStr(x,y,str);
  font.setColor(1);
  font.setFillRectFun(customRect1);
}

// digital hh,mm,ss on lcd1 and lcd2, analog on lcd3
void analogClock() 
{
  clockUpdate();

  font.setFont(&rre_ArialDig47b);
  lcd1.cls();
  font.setDigitMinWd(31);
  font.setFillRectFun(customRectDither1);
  snprintf(buf,5,"%02d",hh);
  outlineD(3,11,buf,14);
  outlineD(96-10,11,":",14);
  lcd1.display();

  lcd2.cls();
  snprintf(buf,3,"%02d",mm);
  outlineD(1,11,buf,14);
  
  font.setFont(&rre_Bold13x20);
  font.setCharMinWd(13);
  snprintf(buf,3,"%02d",ss);
  font.setFillRectFun(customRectDither1);
  outlineD(70-2,36,buf,16);
  lcd2.display();
}


void clockConst()
{
  lcd3.cls();
   // 12 hour lines
  if(rline0>0)
  for(int i=0; i<360; i+=30) {
    sx = fastCos(i-90);
    sy = fastSin(i-90);
    xs = cx+sx*rline0/MAXSIN;
    ys = cy+sy*rline0/MAXSIN;
    xe = cx+sx*rline1/MAXSIN;
    ye = cy+sy*rline1/MAXSIN;
    lcd3.drawLine(xs, ys, xe, ye, 1);
  }

  // 60 second dots
  if(rdot>0)
  for(int i=0; i<360; i+=6) {
    sx = fastCos(i-90);
    sy = fastSin(i-90);
    xs = cx+sx*rdot/MAXSIN;
    ys = cy+sy*rdot/MAXSIN;
    lcd3.drawPixel(xs, ys, 1);
    
    // 4 quadrant dots
    if(qdots && i%90==0) lcd3.fillCircle(xs, ys, 1, 1);
  }
}

void clockUpdate() 
{
  if(millis()-ms<1000 && !start) return;
  ms = millis();
  clockConst();
  tick();

  sdeg = ss*6;
  mdeg = mm*6+sdeg/60;
  hdeg = hh*30+mdeg/12;
  hx = fastCos(hdeg-90);
  hy = fastSin(hdeg-90);
  mx = fastCos(mdeg-90);
  my = fastSin(mdeg-90);
  sx = fastCos(sdeg-90);
  sy = fastSin(sdeg-90);

  if(ss==0 || start) {
    start = 0;
    // clear hour and minute hand positions every minute
    lcd3.drawLine(ohx,ohy, cx,cy, 0);
    ohx = cx+hx*rhr/MAXSIN;
    ohy = cy+hy*rhr/MAXSIN;
    lcd3.drawLine(omx,omy, cx,cy, 0);
    omx = cx+mx*rmin/MAXSIN;
    omy = cy+my*rmin/MAXSIN;
  }

  // new hand positions
  lcd3.drawLine(osx,osy, cx,cy, 0);
  osx = cx+sx*rsec/MAXSIN;
  osy = cy+sy*rsec/MAXSIN;
  lcd3.drawLine(ohx,ohy, cx,cy, 1);
  lcd3.drawLine(omx,omy, cx,cy, 1);
  lcd3.drawLine(osx,osy, cx,cy, 1);
  lcd3.fillCircle(cx,cy, 1, 1);

  if(hrNum>0)
  for(int i=0; i<12; i+=hrNum) {  // +=3 for 12,3,6,9
    sx = fastCos(i*30-90);
    sy = fastSin(i*30-90);
    xs = cx+sx*rnum/MAXSIN;
    ys = cy+sy*rnum/MAXSIN;
    snprintf(buf,3,"%d",i==0?12:i);
    lcd3.printStr(i==0?xs-3:xs-1,ys-3,buf);
  }
  lcd3.display();
}

