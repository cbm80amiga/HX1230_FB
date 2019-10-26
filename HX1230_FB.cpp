#include "HX1230_FB.h"

#define fontbyte(x) pgm_read_byte(&cfont.font[x])

#include <SPI.h>

#define CS_IDLE     digitalWrite(csPin, HIGH)
#define CS_ACTIVE   digitalWrite(csPin, LOW)

const uint8_t initData[] PROGMEM = {
    HX1230_POWER_ON,
    HX1230_CONTRAST+16,
    HX1230_INVERT_OFF,
    HX1230_DISPLAY_NORMAL,
    HX1230_DISPLAY_ON,
    HX1230_SCAN_START_LINE+0,
    HX1230_PAGE_ADDR+0,
    HX1230_COL_ADDR_H+0,
    HX1230_COL_ADDR_L+0 
};
// ----------------------------------------------------------------
HX1230_FB::HX1230_FB(uint8_t rst, uint8_t cs)
{
  rstPin  = rst;
  csPin   = cs;
}
// ----------------------------------------------------------------
void HX1230_FB::init()
{
  scrWd=SCR_WD;
  scrHt=SCR_HT/8;
  isNumberFun = &isNumber;
  cr = 0;
  cfont.font = NULL;
  dualChar = 0;

  pinMode(csPin, OUTPUT);
  if(rstPin<255) {
    pinMode(rstPin, OUTPUT);
    digitalWrite(rstPin, HIGH);
    delay(50);
    digitalWrite(rstPin, LOW);
    delay(5);
    digitalWrite(rstPin, HIGH);
    delay(10);
  }
  
  SPI.begin();
#ifdef __AVR__
  SPI.setClockDivider(SPI_CLOCK_DIV2);
  //SPI.setDataMode(SPI_MODE0);
#endif
  
  CS_ACTIVE;
  for(int i=0; i<sizeof(initData); i++) sendCmd(pgm_read_byte(initData+i));
  CS_IDLE;
}
// ----------------------------------------------------------------
// dc=0 -> command
// dc=1 -> data
// hardware SPI: 9 bits are sent in 2 bytes, still faster than software implementation (108-180fps)
inline void HX1230_FB::sendSPI(uint8_t v, uint8_t dc)
{
  CS_ACTIVE;
  SPI.transfer((v>>1)|dc);
  SPI.transfer(v<<7);
  CS_IDLE;
}
// ----------------------------------------------------------------
inline void HX1230_FB::sendCmd(uint8_t cmd)
{
  sendSPI(cmd,CMD);
}
// ----------------------------------------------------------------
inline void HX1230_FB::sendData(uint8_t data)
{
  sendSPI(data,DAT);
}
// ----------------------------------------------------------------
void HX1230_FB::gotoXY(byte x, byte y)
{
  CS_ACTIVE;
  sendCmd(HX1230_PAGE_ADDR | y);
  sendCmd(HX1230_COL_ADDR_H | (x >> 4));
  sendCmd(HX1230_COL_ADDR_L | (x & 0xf));
  CS_IDLE;
}
// ----------------------------------------------------------------
void HX1230_FB::sleep(bool mode)
{
  if(mode) {
    cls();
    display();
    CS_ACTIVE;
    sendCmd(HX1230_DISPLAY_OFF); // power down
    sendCmd(HX1230_DISPLAY_TEST); // all pixels on (saves power)
    CS_IDLE;
  } else {
    CS_ACTIVE;
    for(int i=0; i<sizeof(initData); i++) sendCmd(pgm_read_byte(initData+i));
    CS_IDLE;
  }
}
// ----------------------------------------------------------------
// 0..31
void HX1230_FB::setContrast(byte val)
{
  CS_ACTIVE;
  sendCmd(HX1230_CONTRAST|(val&0x1f));
  CS_IDLE;
}
// ----------------------------------------------------------------
// 0..63
void HX1230_FB::setScroll(byte val)
{
  CS_ACTIVE;
  sendCmd(HX1230_SCAN_START_LINE|(val&0x3f));
  CS_IDLE;
}
// ----------------------------------------------------------------
// 0..3
void HX1230_FB::setRotate(int mode)
{
  CS_ACTIVE;
  if(mode==2) { // supported rotate 180 deg only
    sendCmd(HX1230_SEG_REMAP);
    sendCmd(HX1230_COM_REMAP);
  } else {
    sendCmd(HX1230_SEG_NORMAL);
    sendCmd(HX1230_COM_NORMAL);
  }
  CS_IDLE;
}
// ----------------------------------------------------------------
void HX1230_FB::displayInvert(bool mode)
{
  CS_ACTIVE;
  sendCmd(mode ? HX1230_INVERT_ON : HX1230_INVERT_OFF);
  CS_IDLE;
}
// ----------------------------------------------------------------
void HX1230_FB::displayOn(bool mode)
{
  CS_ACTIVE;
  sendCmd(mode ? HX1230_DISPLAY_ON : HX1230_DISPLAY_OFF);
  CS_IDLE;
}
// ----------------------------------------------------------------
// val=HX1230_POWER_ON, HX1230_POWER_OFF, HX1230_DISPLAY_NORMAL, HX1230_DISPLAY_TEST
//     HX1230_INVERT_OFF, HX1230_INVERT_ON, HX1230_DISPLAY_ON, HX1230_DISPLAY_OFF
void HX1230_FB::displayMode(byte val)
{
  CS_ACTIVE;
  sendCmd(val);
  CS_IDLE;
}
// ----------------------------------------------------------------
void HX1230_FB::display()
{
  gotoXY(0,0);
  CS_ACTIVE;
  for(int i=0; i<SCR_WD*SCR_HT8; i+=8) {
    sendSPI(scr[i+0],DAT); sendSPI(scr[i+1],DAT);
    sendSPI(scr[i+2],DAT); sendSPI(scr[i+3],DAT);
    sendSPI(scr[i+4],DAT); sendSPI(scr[i+5],DAT);
    sendSPI(scr[i+6],DAT); sendSPI(scr[i+7],DAT);
  }
  CS_IDLE;
}
// ----------------------------------------------------------------
// copy only part of framebuffer
void HX1230_FB::copy(uint8_t x, uint8_t y8, uint8_t wd, uint8_t ht8)
{
  uint8_t wdb = wd;
  for(int i=0; i<ht8; i++) {
    gotoXY(x,y8+i);
    CS_ACTIVE;
    for(int x=0; x<wd; x++) sendSPI(*(scr+wdb*i+x),DAT);
    CS_IDLE;
  }
}
// ----------------------------------------------------------------
// ----------------------------------------------------------------
void HX1230_FB::cls()
{
  memset(scr,0,SCR_WD*SCR_HT8);
}
// ----------------------------------------------------------------
void HX1230_FB::drawPixel(uint8_t x, uint8_t y, uint8_t col) 
{
  if(x>=SCR_WD || y>=SCR_HT) return;
  switch(col) {
    case 1: scr[(y/8)*scrWd+x] |=   (1 << (y&7)); break;
    case 0: scr[(y/8)*scrWd+x] &=  ~(1 << (y&7)); break;
    case 2: scr[(y/8)*scrWd+x] ^=   (1 << (y&7)); break;
  }
}
// ----------------------------------------------------------------
void HX1230_FB::drawLine(int8_t x0, int8_t y0, int8_t x1, int8_t y1, uint8_t col)
{
  int dx = abs(x1-x0);
  int dy = abs(y1-y0);
  int sx = (x0 < x1) ? 1 : -1;
  int sy = (y0 < y1) ? 1 : -1;
  int err = dx - dy;
 
  while(1) {
    //if(x0>=0 && y0>=0) 
      drawPixel(x0, y0, col);
    if(x0==x1 && y0==y1) return;
    int err2 = err+err;
    if(err2>-dy) { err-=dy; x0+=sx; }
    if(err2< dx) { err+=dx; y0+=sy; }
  }
}
// ----------------------------------------------------------------
void HX1230_FB::drawLineH(uint8_t x0, uint8_t x1, uint8_t y, uint8_t col)
{
  if(x1>x0) for(uint8_t x=x0; x<=x1; x++) drawPixel(x,y,col);
  else      for(uint8_t x=x1; x<=x0; x++) drawPixel(x,y,col);
}
// ----------------------------------------------------------------
void HX1230_FB::drawLineV(uint8_t x, uint8_t y0, uint8_t y1, uint8_t col)
{
  if(y1>y0) for(uint8_t y=y0; y<=y1; y++) drawPixel(x,y,col);
  else      for(uint8_t y=y1; y<=y0; y++) drawPixel(x,y,col);
}
// ----------------------------------------------------------------
// about 4x faster than regular drawLineH
void HX1230_FB::drawLineHfast(uint8_t x0, uint8_t x1, uint8_t y, uint8_t col)
{
  uint8_t mask;
  if(x1<x0) { mask=x0; x0=x1; x1=mask; } // swap
  mask = 1 << (y&7);
  switch(col) {
    case 1: for(int x=x0; x<=x1; x++) scr[(y/8)*scrWd+x] |= mask;   break;
    case 0: for(int x=x0; x<=x1; x++) scr[(y/8)*scrWd+x] &= ~mask;  break;
    case 2: for(int x=x0; x<=x1; x++) scr[(y/8)*scrWd+x] ^= mask;   break;
  }
}
// ----------------------------------------------------------------
// limited to pattern #8
void HX1230_FB::drawLineHfastD(uint8_t x0, uint8_t x1, uint8_t y, uint8_t col)
{
  uint8_t mask;
  if(x1<x0) { mask=x0; x0=x1; x1=mask; } // swap
  if(((x0&1)==1 && (y&1)==0) || ((x0&1)==0 && (y&1)==1)) x0++;
  mask = 1 << (y&7);
  switch(col) {
    case 1: for(int x=x0; x<=x1; x+=2) scr[(y/8)*scrWd+x] |= mask;   break;
    case 0: for(int x=x0; x<=x1; x+=2) scr[(y/8)*scrWd+x] &= ~mask;  break;
    case 2: for(int x=x0; x<=x1; x+=2) scr[(y/8)*scrWd+x] ^= mask;   break;
  }
}
// ----------------------------------------------------------------
byte HX1230_FB::ystab[8]={0xff,0xfe,0xfc,0xf8,0xf0,0xe0,0xc0,0x80};
byte HX1230_FB::yetab[8]={0x01,0x03,0x07,0x0f,0x1f,0x3f,0x7f,0xff};
byte HX1230_FB::pattern[4]={0xaa,0x55,0xaa,0x55};
// about 40x faster than regular drawLineV
void HX1230_FB::drawLineVfast(uint8_t x, uint8_t y0, uint8_t y1, uint8_t col)
{
  if(x<0 || x>SCR_WD) return;
  int y8s,y8e;
  if(y1<y0) { y8s=y1; y1=y0; y0=y8s; } // swap
  if(y0<0) y0=0;
  if(y1>=SCR_WD) y1=SCR_WD-1;
  y8s=y0/8;
  y8e=y1/8;

  switch(col) {
    case 1: 
      if(y8s==y8e) scr[y8s*scrWd+x]|=(ystab[y0&7] & yetab[y1&7]);
      else { scr[y8s*scrWd+x]|=ystab[y0&7]; scr[y8e*scrWd+x]|=yetab[y1&7]; }
      for(int y=y8s+1; y<y8e; y++) scr[y*scrWd+x]=0xff;
      break;
    case 0:
      if(y8s==y8e) scr[y8s*scrWd+x]&=~(ystab[y0&7] & yetab[y1&7]);
      else { scr[y8s*scrWd+x]&=~ystab[y0&7]; scr[y8e*scrWd+x]&=~yetab[y1&7]; }
      for(int y=y8s+1; y<y8e; y++) scr[y*scrWd+x]=0x00;
      break;
    case 2: 
      if(y8s==y8e) scr[y8s*scrWd+x]^=(ystab[y0&7] & yetab[y1&7]);
      else { scr[y8s*scrWd+x]^=ystab[y0&7]; scr[y8e*scrWd+x]^=yetab[y1&7]; }
      for(int y=y8s+1; y<y8e; y++) scr[y*scrWd+x]^=0xff;
      break;
  }
}
// ----------------------------------------------------------------
// dithered version
void HX1230_FB::drawLineVfastD(uint8_t x, uint8_t y0, uint8_t y1, uint8_t col)
{
  int y8s,y8e;
  if(y1<y0) { y8s=y1; y1=y0; y0=y8s; } // swap
  y8s=y0/8;
  y8e=y1/8;

  switch(col) {
    case 1: 
      if(y8s==y8e) scr[y8s*scrWd+x]|=(ystab[y0&7] & yetab[y1&7] & pattern[x&3]);
      else { scr[y8s*scrWd+x]|=(ystab[y0&7] & pattern[x&3]); scr[y8e*scrWd+x]|=(yetab[y1&7] & pattern[x&3]); }
      for(int y=y8s+1; y<y8e; y++) scr[y*scrWd+x]|=pattern[x&3];
      break;
    case 0:
      if(y8s==y8e) scr[y8s*scrWd+x]&=~(ystab[y0&7] & yetab[y1&7] & pattern[x&3]);
      else { scr[y8s*scrWd+x]&=~(ystab[y0&7] & pattern[x&3]); scr[y8e*scrWd+x]&=~(yetab[y1&7] & pattern[x&3]); }
      for(int y=y8s+1; y<y8e; y++) scr[y*scrWd+x]&=~pattern[x&3];
      break;
    case 2: 
      if(y8s==y8e) scr[y8s*scrWd+x]^=(ystab[y0&7] & yetab[y1&7] & pattern[x&3]);
      else { scr[y8s*scrWd+x]^=(ystab[y0&7] & pattern[x&3]); scr[y8e*scrWd+x]^=(yetab[y1&7] & pattern[x&3]); }
      for(int y=y8s+1; y<y8e; y++) scr[y*scrWd+x]^=pattern[x&3];
      break;
  }
}
// ----------------------------------------------------------------
void HX1230_FB::drawRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t col)
{
  if(x>=SCR_WD || y>=SCR_HT) return;
  byte drawVright=1;
  if(x+w>SCR_WD) { w=SCR_WD-x; drawVright=0; }
  if(y+h>SCR_HT) h=SCR_HT-y; else drawLineHfast(x, x+w-1, y+h-1,col);
  drawLineHfast(x, x+w-1, y,col);
  drawLineVfast(x,    y+1, y+h-2,col);
  if(drawVright) drawLineVfast(x+w-1,y+1, y+h-2,col);
}
// ----------------------------------------------------------------
// dithered version (50% of brightness)
void HX1230_FB::drawRectD(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t col)
{
  if(x>=SCR_WD || y>=SCR_HT) return;
  byte drawVright=1;
  if(x+w>SCR_WD) { w=SCR_WD-x; drawVright=0; }
  if(y+h>SCR_HT) h=SCR_HT-y; else drawLineHfastD(x, x+w-1, y+h-1,col);
  drawLineHfastD(x, x+w-1, y,col);
  drawLineVfastD(x,    y+1, y+h-2,col);
  if(drawVright) drawLineVfastD(x+w-1,y+1, y+h-2,col);
}
// ----------------------------------------------------------------
void HX1230_FB::fillRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t col)
{
  if(x>=SCR_WD || y>=SCR_HT || w<=0 || h<=0) return;
  if(x+w>SCR_WD) w=SCR_WD-x;
  if(y+h>SCR_HT) h=SCR_HT-y;
  for(int i=x;i<x+w;i++) drawLineVfast(i,y,y+h-1,col);
}
// ----------------------------------------------------------------
// dithered version (50% of brightness)
void HX1230_FB::fillRectD(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t col)
{
  if(x>=SCR_WD || y>=SCR_HT || w<=0 || h<=0) return;
  if(x+w>=SCR_WD) w=SCR_WD-x;
  if(y+h>=SCR_HT) h=SCR_HT-y;
  for(int i=x;i<x+w;i++) drawLineVfastD(i,y,y+h-1,col);
}
// ----------------------------------------------------------------
// circle
void HX1230_FB::drawCircle(uint8_t x0, uint8_t y0, uint8_t radius, uint8_t col)
{
  int f = 1 - (int)radius;
  int ddF_x = 1;
  int ddF_y = -2 * (int)radius;
  int x = 0;
  int y = radius;
 
  drawPixel(x0, y0 + radius, col);
  drawPixel(x0, y0 - radius, col);
  drawPixel(x0 + radius, y0, col);
  drawPixel(x0 - radius, y0, col);
 
  while(x < y) {
    if(f >= 0) {
      y--; ddF_y += 2; f += ddF_y;
    }
    x++; ddF_x += 2; f += ddF_x;    
    drawPixel(x0 + x, y0 + y, col);
    drawPixel(x0 - x, y0 + y, col);
    drawPixel(x0 + x, y0 - y, col);
    drawPixel(x0 - x, y0 - y, col);
    drawPixel(x0 + y, y0 + x, col);
    drawPixel(x0 - y, y0 + x, col);
    drawPixel(x0 + y, y0 - x, col);
    drawPixel(x0 - y, y0 - x, col);
  }
}
// ----------------------------------------------------------------
void HX1230_FB::fillCircle(uint8_t x0, uint8_t y0, uint8_t r, uint8_t col)
{
  drawLineVfast(x0, y0-r, y0-r+2*r+1, col);
  int16_t f     = 1 - r;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * r;
  int16_t x     = 0;
  int16_t y     = r;

  while (x<y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f     += ddF_y;
    }
    x++;
    ddF_x += 2;
    f     += ddF_x;
    drawLineVfast(x0+x, y0-y, y0-y+2*y+1, col);
    drawLineVfast(x0+y, y0-x, y0-x+2*x+1, col);
    drawLineVfast(x0-x, y0-y, y0-y+2*y+1, col);
    drawLineVfast(x0-y, y0-x, y0-x+2*x+1, col);
  }
}
// ----------------------------------------------------------------
// dithered version (50% of brightness)
void HX1230_FB::fillCircleD(uint8_t x0, uint8_t y0, uint8_t r, uint8_t col)
{
  drawLineVfastD(x0, y0-r, y0-r+2*r+1, col);
  int16_t f     = 1 - r;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * r;
  int16_t x     = 0;
  int16_t y     = r;

  while (x<y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f     += ddF_y;
    }
    x++;
    ddF_x += 2;
    f     += ddF_x;
    drawLineVfastD(x0+x, y0-y, y0-y+2*y+1, col);
    drawLineVfastD(x0+y, y0-x, y0-x+2*x+1, col);
    drawLineVfastD(x0-x, y0-y, y0-y+2*y+1, col);
    drawLineVfastD(x0-y, y0-x, y0-x+2*x+1, col);
  }
}
// ----------------------------------------------------------------
void HX1230_FB::drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color)
{
  drawLine(x0, y0, x1, y1, color);
  drawLine(x1, y1, x2, y2, color);
  drawLine(x2, y2, x0, y0, color);
}
#define swap(a, b) { int16_t t = a; a = b; b = t; }
// ----------------------------------------------------------------
void HX1230_FB::fillTriangle( int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color)
{
  int16_t a, b, y, last;

  // Sort coordinates by Y order (y2 >= y1 >= y0)
  if (y0 > y1) { swap(y0, y1); swap(x0, x1); }
  if (y1 > y2) { swap(y2, y1); swap(x2, x1); }
  if (y0 > y1) { swap(y0, y1); swap(x0, x1); }

  if(y0 == y2) { // Handle awkward all-on-same-line case as its own thing
    a = b = x0;
    if(x1 < a)      a = x1;
    else if(x1 > b) b = x1;
    if(x2 < a)      a = x2;
    else if(x2 > b) b = x2;
    drawLineHfast(a, b, y0, color);
    return;
  }

  int16_t
    dx01 = x1 - x0,
    dy01 = y1 - y0,
    dx02 = x2 - x0,
    dy02 = y2 - y0,
    dx12 = x2 - x1,
    dy12 = y2 - y1;
  long
    sa   = 0,
    sb   = 0;

  // For upper part of triangle, find scanline crossings for segments
  // 0-1 and 0-2.  If y1=y2 (flat-bottomed triangle), the scanline y1
  // is included here (and second loop will be skipped, avoiding a /0
  // error there), otherwise scanline y1 is skipped here and handled
  // in the second loop...which also avoids a /0 error here if y0=y1
  // (flat-topped triangle).
  if(y1 == y2) last = y1;   // Include y1 scanline
  else         last = y1-1; // Skip it

  for(y=y0; y<=last; y++) {
    a   = x0 + sa / dy01;
    b   = x0 + sb / dy02;
    sa += dx01;
    sb += dx02;
    /* longhand:
    a = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
    b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
    */
    if(a > b) swap(a,b);
    drawLineHfast(a, b, y, color);
  }

  // For lower part of triangle, find scanline crossings for segments
  // 0-2 and 1-2.  This loop is skipped if y1=y2.
  sa = dx12 * (y - y1);
  sb = dx02 * (y - y0);
  for(; y<=y2; y++) {
    a   = x1 + sa / dy12;
    b   = x0 + sb / dy02;
    sa += dx12;
    sb += dx02;
    /* longhand:
    a = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
    b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
    */
    if(a > b) swap(a,b);
    drawLineHfast(a, b, y, color);
  }
}
// ----------------------------------------------------------------
void HX1230_FB::fillTriangleD( int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color)
{
  int16_t a, b, y, last;

  // Sort coordinates by Y order (y2 >= y1 >= y0)
  if (y0 > y1) { swap(y0, y1); swap(x0, x1); }
  if (y1 > y2) { swap(y2, y1); swap(x2, x1); }
  if (y0 > y1) { swap(y0, y1); swap(x0, x1); }

  if(y0 == y2) { // Handle awkward all-on-same-line case as its own thing
    a = b = x0;
    if(x1 < a)      a = x1;
    else if(x1 > b) b = x1;
    if(x2 < a)      a = x2;
    else if(x2 > b) b = x2;
    drawLineHfastD(a, b, y0, color);
    return;
  }

  int16_t
    dx01 = x1 - x0,
    dy01 = y1 - y0,
    dx02 = x2 - x0,
    dy02 = y2 - y0,
    dx12 = x2 - x1,
    dy12 = y2 - y1;
  long
    sa   = 0,
    sb   = 0;

  // For upper part of triangle, find scanline crossings for segments
  // 0-1 and 0-2.  If y1=y2 (flat-bottomed triangle), the scanline y1
  // is included here (and second loop will be skipped, avoiding a /0
  // error there), otherwise scanline y1 is skipped here and handled
  // in the second loop...which also avoids a /0 error here if y0=y1
  // (flat-topped triangle).
  if(y1 == y2) last = y1;   // Include y1 scanline
  else         last = y1-1; // Skip it

  for(y=y0; y<=last; y++) {
    a   = x0 + sa / dy01;
    b   = x0 + sb / dy02;
    sa += dx01;
    sb += dx02;
    /* longhand:
    a = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
    b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
    */
    if(a > b) swap(a,b);
    drawLineHfastD(a, b, y, color);
  }

  // For lower part of triangle, find scanline crossings for segments
  // 0-2 and 1-2.  This loop is skipped if y1=y2.
  sa = dx12 * (y - y1);
  sb = dx02 * (y - y0);
  for(; y<=y2; y++) {
    a   = x1 + sa / dy12;
    b   = x0 + sb / dy02;
    sa += dx12;
    sb += dx02;
    /* longhand:
    a = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
    b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
    */
    if(a > b) swap(a,b);
    drawLineHfastD(a, b, y, color);
  }
}
// ----------------------------------------------------------------
const byte HX1230_FB::ditherTab[4*17] PROGMEM = {
  0x00,0x00,0x00,0x00, // 0

  0x88,0x00,0x00,0x00, // 1
  0x88,0x00,0x22,0x00, // 2
  0x88,0x00,0xaa,0x00, // 3
  0xaa,0x00,0xaa,0x00, // 4
  0xaa,0x44,0xaa,0x00, // 5
  0xaa,0x44,0xaa,0x11, // 6
  0xaa,0x44,0xaa,0x55, // 7

  0xaa,0x55,0xaa,0x55, // 8

  0xaa,0xdd,0xaa,0x55, // 9
  0xaa,0xdd,0xaa,0x77, // 10
  0xaa,0xdd,0xaa,0xff, // 11
  0xaa,0xff,0xaa,0xff, // 12
  0xbb,0xff,0xaa,0xff, // 13
  0xbb,0xff,0xee,0xff, // 14
  0xbb,0xff,0xff,0xff, // 15

  0xff,0xff,0xff,0xff  // 16
};

void HX1230_FB::setDither(uint8_t s)
{
  if(s>16) s=16;;
  pattern[0] = pgm_read_byte(ditherTab+s*4+0);
  pattern[1] = pgm_read_byte(ditherTab+s*4+1);
  pattern[2] = pgm_read_byte(ditherTab+s*4+2);
  pattern[3] = pgm_read_byte(ditherTab+s*4+3);
}
// ----------------------------------------------------------------
#define ALIGNMENT \
  if(x==-1) x = SCR_WD-w; \
  else if(x<0) x = (SCR_WD-w)/2; \
  if(x<0) x=0; \
  if(x>=SCR_WD || y>=SCR_HT) return 0; \
  if(x+w>SCR_WD) w = SCR_WD-x; \
  if(y+h>SCR_HT) h = SCR_HT-y
// ----------------------------------------------------------------

int HX1230_FB::drawBitmap(const uint8_t *bmp, int x, uint8_t y, uint8_t w, uint8_t h)
{
  uint8_t wdb = w;
  ALIGNMENT;
  byte i,y8,d,b,ht8=(h+7)/8;
  for(y8=0; y8<ht8; y8++) {
    for(i=0; i<w; i++) {
      d = pgm_read_byte(bmp+wdb*y8+i);
      int lastbit = h - y8 * 8;
      if (lastbit > 8) lastbit = 8;
      for(b=0; b<lastbit; b++) {
         if(d & 1) scr[((y+y8*8+b)/8)*scrWd+x+i] |= (1 << ((y+y8*8+b)&7));
         d>>=1;
      }
    }
  }
  return x+w;
}
// ----------------------------------------------------------------
int HX1230_FB::drawBitmap(const uint8_t *bmp, int x, uint8_t y)
{
  uint8_t w = pgm_read_byte(bmp+0);
  uint8_t h = pgm_read_byte(bmp+1);
  return drawBitmap(bmp+2, x, y, w, h);
}
// ----------------------------------------------------------------
// text rendering
// ----------------------------------------------------------------
void HX1230_FB::setFont(const uint8_t* font)
{
  cfont.font = font;
  cfont.xSize = fontbyte(0);
  cfont.ySize = fontbyte(1);
  cfont.firstCh = fontbyte(2);
  cfont.lastCh  = fontbyte(3);
  cfont.minDigitWd = 0;
  cfont.minCharWd = 0;
  isNumberFun = &isNumber;
  spacing = 1;
  cr = 0;
  invertCh = 0;
}
// ----------------------------------------------------------------
int HX1230_FB::charWidth(uint8_t c, bool last)
{
  c = convertPolish(c);
  if(c < cfont.firstCh || c > cfont.lastCh)
    return c==' ' ?  1 + cfont.xSize/2 : 0;
  if (cfont.xSize > 0) return cfont.xSize;
  int ys8=(cfont.ySize+7)/8;
  int idx = 4 + (c-cfont.firstCh)*(-cfont.xSize*ys8+1);
  int wd = pgm_read_byte(cfont.font + idx);
  int wdL = 0, wdR = spacing; // default spacing before and behind char
  if((*isNumberFun)(c)) {
    if(cfont.minDigitWd>wd) {
      wdL = (cfont.minDigitWd-wd)/2;
      wdR += (cfont.minDigitWd-wd-wdL);
    }
  } else
  if(cfont.minCharWd>wd) {
    wdL = (cfont.minCharWd-wd)/2;
    wdR += (cfont.minCharWd-wd-wdL);
  }
  return last ? wd+wdL+wdR : wd+wdL+wdR-spacing;  // last!=0 -> get rid of last empty columns 
}
// ----------------------------------------------------------------
int HX1230_FB::strWidth(char *str)
{
  int wd = 0;
  while (*str) wd += charWidth(*str++);
  return wd;
}
// ----------------------------------------------------------------
int HX1230_FB::printChar(int xpos, int ypos, unsigned char c)
{
  if(xpos >= SCR_WD || ypos >= SCR_HT)  return 0;
  int fht8 = (cfont.ySize + 7) / 8, wd, fwd = cfont.xSize;
  if(fwd < 0)  fwd = -fwd;

  c = convertPolish(c);
  if(c < cfont.firstCh || c > cfont.lastCh)  return c==' ' ?  1 + fwd/2 : 0;

  int x,y8,b,cdata = (c - cfont.firstCh) * (fwd*fht8+1) + 4;
  byte d;
  wd = fontbyte(cdata++);
  int wdL = 0, wdR = spacing;
  if((*isNumberFun)(c)) {
    if(cfont.minDigitWd>wd) {
      wdL = (cfont.minDigitWd-wd)/2;
      wdR += (cfont.minDigitWd-wd-wdL);
    }
  } else
  if(cfont.minCharWd>wd) {
    wdL = (cfont.minCharWd-wd)/2;
    wdR += (cfont.minCharWd-wd-wdL);
  }
  if(xpos+wd+wdL+wdR>SCR_WD) wdR = max(SCR_WD-xpos-wdL-wd, 0);
  if(xpos+wd+wdL+wdR>SCR_WD) wd  = max(SCR_WD-xpos-wdL, 0);
  if(xpos+wd+wdL+wdR>SCR_WD) wdL = max(SCR_WD-xpos, 0);

  for(x=0; x<wd; x++) {
    //byte mask = 1 << ((xpos+x+wdL)&7);
    for(y8=0; y8<fht8; y8++) {
      d = fontbyte(cdata+x*fht8+y8);
      int lastbit = cfont.ySize - y8 * 8;
      if (lastbit > 8) lastbit = 8;
      for(b=0; b<lastbit; b++) {
         if(d & 1) scr[((ypos+y8*8+b)/8)*scrWd+xpos+x] |= 1<<((ypos+y8*8+b)&7);  //drawPixel(xpos+x, ypos+y8*8+b, 1);
         d>>=1;
      }
    }
  }
  return wd+wdR+wdL;
}
// ----------------------------------------------------------------
int HX1230_FB::printStr(int xpos, int ypos, char *str)
{
  unsigned char ch;
  int stl, row;
  int x = xpos;
  int y = ypos;
  int wd = strWidth(str);

  if(x==-1) // right = -1
    x = SCR_WD - wd;
  else if(x<0) // center = -2
    x = (SCR_WD - wd) / 2;
  if(x<0) x = 0; // left

  while(*str) {
    int wd = printChar(x,y,*str++);
    x+=wd;
    if(cr && x>=SCR_WD) { 
      x=0; 
      y+=cfont.ySize; 
      if(y>SCR_HT) y = 0;
    }
  }
  if(invertCh) fillRect(xpos,x-1,y,y+cfont.ySize+1,2);
  return x;
}
// ----------------------------------------------------------------
bool HX1230_FB::isNumber(uint8_t ch)
{
  return isdigit(ch) || ch==' ';
}
// ---------------------------------
bool HX1230_FB::isNumberExt(uint8_t ch)
{
  return isdigit(ch) || ch=='-' || ch=='+' || ch=='.' || ch==' ';
}
// ----------------------------------------------------------------
unsigned char HX1230_FB::convertPolish(unsigned char _c)
{
  unsigned char pl, c = _c;
  if(c==196 || c==197 || c==195) {
	  dualChar = c;
    return 0;
  }
  if(dualChar) { // UTF8 coding
    switch(_c) {
      case 133: pl = 1+9; break; // '�'
      case 135: pl = 2+9; break; // '�'
      case 153: pl = 3+9; break; // '�'
      case 130: pl = 4+9; break; // '�'
      case 132: pl = dualChar==197 ? 5+9 : 1; break; // '�' and '�'
      case 179: pl = 6+9; break; // '�'
      case 155: pl = 7+9; break; // '�'
      case 186: pl = 8+9; break; // '�'
      case 188: pl = 9+9; break; // '�'
      //case 132: pl = 1; break; // '�'
      case 134: pl = 2; break; // '�'
      case 152: pl = 3; break; // '�'
      case 129: pl = 4; break; // '�'
      case 131: pl = 5; break; // '�'
      case 147: pl = 6; break; // '�'
      case 154: pl = 7; break; // '�'
      case 185: pl = 8; break; // '�'
      case 187: pl = 9; break; // '�'
      default:  return c; break;
    }
    dualChar = 0;
  } else   
  switch(_c) {  // Windows coding
    case 165: pl = 1; break; // �
    case 198: pl = 2; break; // �
    case 202: pl = 3; break; // �
    case 163: pl = 4; break; // �
    case 209: pl = 5; break; // �
    case 211: pl = 6; break; // �
    case 140: pl = 7; break; // �
    case 143: pl = 8; break; // �
    case 175: pl = 9; break; // �
    case 185: pl = 10; break; // �
    case 230: pl = 11; break; // �
    case 234: pl = 12; break; // �
    case 179: pl = 13; break; // �
    case 241: pl = 14; break; // �
    case 243: pl = 15; break; // �
    case 156: pl = 16; break; // �
    case 159: pl = 17; break; // �
    case 191: pl = 18; break; // �
    default:  return c; break;
  }
  return pl+'~'+1;
}
// ---------------------------------