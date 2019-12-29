# HX1230_FB
Fast 9-bit SPI graphics library for HX1230 96x68 LCD (aka Nokia 1202, STE2007)

Uses 96x9 bytes of RAM as frame buffer memory. Most common graphics primitives implemented. Dithering available.

YouTube videos:
https://www.youtube.com/playlist?list=PLxb1losWErZ5eNiQAwB75pXTec5KzX_Cf


## Features

- proportional fonts support built-in (requires fonts from PropFonts library https://github.com/cbm80amiga/PropFonts)
- simple primitives
  - pixels
  - lines
  - rectangles
  - filled rectangles
  - circles
  - filled circles
  - triangles
  - filled triangles
- fast ordered dithering (17 patterns)
- ultra fast horizontal and vertical line drawing
- bitmaps drawing
- example programs

## Connections

|HX1230 pin|Pin name| Arduino|
|--|--|--|
|#1| RST    |D6 or any digital|
|#2| CE  |D7 or any digital|
|#3| N/C     ||
|#4| DIN | D11/MOSI |
|#5| CLK  |D13/SCK|
|#6| VCC| 3.3V|
|#7| BL| 3.3V or via resistor to any digital pin|
|#8 |GND|GND|

If you find it useful and want to buy me a coffee or a beer:

https://www.paypal.me/cbm80amiga
