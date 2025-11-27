#ifndef SHAPES96_H
#define SHAPES96_H
//
// OLED96
// Library for accessing the 0.96" SSD1306 128x64 OLED display
// Written by Larry Bank (bitbank@pobox.com)
// Copyright (c) 2017 BitBank Software, Inc.
// Project started 1/15/2017
//
// Enhanced by Kelly Wiles 2025/11/27, I added shapes.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//    http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//===========================================================================

// OLED type for init function
enum {
  OLED_128x32 = 1,
  OLED_128x64,
  OLED_132x64,
  OLED_64x32
};

typedef enum
{
   FONT_NORMAL=0,	// 8x8
   FONT_BIG,		// 16x24
   FONT_SMALL		// 6x8
} FONTSIZE;

// Initialize the OLED96 library for a specific I2C address
// Optionally enable inverted or flipped mode
// returns 0 for success, 1 for failure
//
int oledInit(int iChannel, int iAddress, int iType, int bFlip, int bInvert);

// Turns off the display and closes the I2C handle
void oledShutdown(void);

// Fills the display with the byte pattern
int oledFill(unsigned char ucPattern);

// Write a text string to the display at x (column 0-127) and y (row 0-7)
// bLarge = 0 - 8x8 font, bLarge = 1 - 16x24 font
int oledWriteString(int x, int y, char *szText, int bLarge);
int oledPrintf(int x, int y, char *szText, int bLarge, ...);

// Sets a pixel to On (1) or Off (0)
// Coordinate system is pixels, not text rows (0-127, 0-63)
int oledSetPixel(int x, int y, unsigned char ucPixel);

// Sets the contrast (brightness) level of the display
// Valid values are 0-255 where 0=off and 255=max brightness
int oledSetContrast(unsigned char ucContrast);

int oledCircle(int xc, int yc, int r, unsigned char color);
int oledFilledCircle(int xc, int yc, int r, unsigned char color);
int oledSquare(int x, int y, int size, unsigned char color);
int oledFilledSquare(int x, int y, int size, unsigned char color);
int oledRectangle(int x, int y, int width, int height, unsigned char color);
int oledFilledRectangle(int x, int y, int width, int height, unsigned char color);
int oledEllipse(int xc, int yc, int rx, int ry, unsigned char color);
int oledFilledEllipse(int xc, int yc, int rx, int ry, unsigned char color);
int oledTriangle(int x0, int y0, int x1, int y1, int x2, int y2, unsigned char color);
int oledFilledTriangle(int x0, int y0, int x1, int y1, int x2, int y2, unsigned char color);
int oledLine(int x0, int y0, int x1, int y1, unsigned char color);
int oledPolygon(int *vx, int *vy, int vertices, unsigned char color);
int oledFilledPolygon(int *vx, int *vy, int vertices, unsigned char color);
int oledArc(int xc, int yc, int r, float sa, float ea, unsigned char color);
int oledBezier(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3, unsigned char color);
int oledParabola(int h, int k, float a, int xs, int xe, unsigned char color);
#endif // SHAPES96_H
