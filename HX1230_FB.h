// Fast HX1230 96x68 LCD graphics library with fb
// (C) 2019 by Pawel A. Hernik

/*
  HX1230 96x68 LCD connections (header on bottom, from left):
  #1 RST - D6 or any digital
  #2 CE  - D7 or any digital
  #3 N/C
  #4 DIN - D11/MOSI 
  #5 CLK - D13/SCK
  #6 VCC - 3V3
  #7 BL  - 3V3 or any digital via resistor
  #8 GND - GND
*/

#ifndef _HX1230_FB_H
#define _HX1230_FB_H 

// define for real 9-bit SPI transfer, each 8 bytes are transfered in 9 bytes with D/C bit
// otherwise each 9-bit data is transfered in 2 bytes what seems to be a problem for some HX1230 LCDs
#define REAL_SPI9

#include <Arduino.h>
#include <avr/pgmspace.h>

#define CMD 0x00
#define DAT 0x80

// HX1230 Commands
#define HX1230_POWER_ON         0x2F // internal power supply on
#define HX1230_POWER_OFF        0x28 // internal power supply off
#define HX1230_CONTRAST         0x80 // 0x80 + (0..31)
#define HX1230_SEG_NORMAL       0xA0 // SEG remap normal
#define HX1230_SEG_REMAP        0xA1 // SEG remap reverse (flip horizontal)
#define HX1230_DISPLAY_NORMAL   0xA4 // display ram contents
#define HX1230_DISPLAY_TEST     0xA5 // all pixels on
#define HX1230_INVERT_OFF       0xA6 // not inverted
#define HX1230_INVERT_ON        0xA7 // inverted
#define HX1230_DISPLAY_ON       0XAF // display on
#define HX1230_DISPLAY_OFF      0XAE // display off
#define HX1230_SCAN_START_LINE  0x40 // scrolling 0x40 + (0..63)
#define HX1230_COM_NORMAL       0xC0 // COM remap normal
#define HX1230_COM_REMAP        0xC8 // COM remap reverse (flip vertical)
#define HX1230_SW_RESET         0xE2 // connect RST pin to GND and rely on software reset
#define HX1230_NOP              0xE3 // no operation
#define HX1230_COL_ADDR_H       0x10 // x pos (0..95) 4 MSB
#define HX1230_COL_ADDR_L       0x00 // x pos (0..95) 4 LSB
#define HX1230_PAGE_ADDR        0xB0 // y pos, 8.5 rows (0..8)

#define SCR_WD  96
#define SCR_HT  68
#define SCR_HT8 9  // (SCR_HT+7)/8

#define ALIGN_LEFT    0
#define ALIGN_RIGHT  -1 
#define ALIGN_CENTER -2

#define SET 1
#define CLR 0
#define XOR 2

struct _propFont
{
  const uint8_t* font;
  int8_t xSize;
  uint8_t ySize;
  uint8_t firstCh;
  uint8_t lastCh;
  uint8_t minCharWd;
  uint8_t minDigitWd;
};

// ---------------------------------
class HX1230_FB {
public:
  HX1230_FB(uint8_t rst, uint8_t cs);

  inline void sendSPI(uint8_t v, uint8_t dc) __attribute__((always_inline)); // costs about 350B of flash
  inline void sendCmd(uint8_t cmd);
  inline void sendData(uint8_t data);
  void init();
  void display();
  void copy(uint8_t x, uint8_t y8, uint8_t wd, uint8_t ht8);
  void gotoXY(byte x, byte y);
  void sleep(bool mode=true);
  void setContrast(byte val);
  void setScroll(byte val);
  void displayInvert(bool mode);
  void displayOn(bool mode);
  void displayMode(byte val);
  void setRotate(int mode);

  void cls();
  void clearDisplay() { cls(); }
  void drawPixel(uint8_t x, uint8_t y, uint8_t col);
  void drawLine(int8_t x0, int8_t y0, int8_t x1, int8_t y1, uint8_t col);
  void drawLineH(uint8_t x0, uint8_t x1, uint8_t y, uint8_t col);
  void drawLineV(uint8_t x,  uint8_t y0, uint8_t y1, uint8_t col);
  void drawLineVfast(uint8_t x, uint8_t y0, uint8_t y1, uint8_t col);
  void drawLineVfastD(uint8_t x, uint8_t y0, uint8_t y1, uint8_t col);
  void drawLineHfast(uint8_t x0, uint8_t x1, uint8_t y, uint8_t col);
  void drawLineHfastD(uint8_t x0, uint8_t x1, uint8_t y, uint8_t col);
  void drawRect(uint8_t x0, uint8_t y0, uint8_t w, uint8_t h, uint8_t col);
  void drawRectD(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t col);
  void fillRect(uint8_t x0, uint8_t y0, uint8_t w, uint8_t h, uint8_t col);
  void fillRectD(uint8_t x0, uint8_t y0, uint8_t w, uint8_t h, uint8_t col);
  void drawCircle(uint8_t x0, uint8_t y0, uint8_t radius, uint8_t col);
  void fillCircle(uint8_t x0, uint8_t y0, uint8_t r, uint8_t col);
  void fillCircleD(uint8_t x0, uint8_t y0, uint8_t r, uint8_t col);
  void drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
  void fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
  void fillTriangleD(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
  void setDither(int8_t s);
  int drawBitmap(const uint8_t *bmp, int x, uint8_t y, uint8_t w, uint8_t h);
  int drawBitmap(const uint8_t *bmp, int x, uint8_t y);

  void setFont(const uint8_t* f);
  void setCR(uint8_t _cr) { cr = _cr; }
  void setInvert(uint8_t _inv) { invertCh = _inv; }
  void setFontMinWd(uint8_t wd) { cfont.minCharWd = wd; }
  void setCharMinWd(uint8_t wd) { cfont.minCharWd = wd; }
  void setDigitMinWd(uint8_t wd) { cfont.minDigitWd = wd; }
  int printChar(int xpos, int ypos, unsigned char c);
  int printStr(int xpos, int ypos, char *str);
  int charWidth(uint8_t _ch, bool last=true);
  int strWidth(char *txt);
  unsigned char convertPolish(unsigned char _c);
  static bool isNumber(uint8_t ch);
  static bool isNumberExt(uint8_t ch);
  void setIsNumberFun(bool (*fun)(uint8_t)) { isNumberFun=fun; }
  
public:
  static byte scr[SCR_WD*SCR_HT8];
  byte scrWd = SCR_WD;
  byte scrHt = SCR_HT8;
  uint8_t csPin, rstPin;

  static byte ystab[8];
  static byte yetab[8];
  static byte pattern[4];
  static const byte ditherTab[4*17];

//private:
  bool (*isNumberFun)(uint8_t ch);
  _propFont  cfont;
  uint8_t cr;  // carriage return mode for printStr
  uint8_t dualChar;
  uint8_t invertCh;
  uint8_t spacing = 1;	
};
#endif

