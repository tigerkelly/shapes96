// OLED SSD1306 using the I2C interface
// Written by Larry Bank (bitbank@pobox.com)
// Copyright (c) 2017 BitBank Software, Inc.
// Project started 1/15/2017
// Modified by Kelly Wiles to add other functions.
//     Added oledPrintf();
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

// The I2C writes (through a file handle) can be single or multiple bytes.
// The write mode stays in effect throughout each call to write()
// To write commands to the OLED controller, start a byte sequence with 0x00,
// to write data, start a byte sequence with 0x40,
// The OLED controller is set to "page mode". This divides the display
// into 8 128x8 "pages" or strips. Each data write advances the output
// automatically to the next address. The bytes are arranged such that the LSB
// is the topmost pixel and the MSB is the bottom.
// The font data comes from another source and must be rotated 90 degrees
// (at init time) to match the orientation of the bits on the display memory.
// A copy of the display memory is maintained by this code so that single pixel
// writes can occur without having to read from the display controller.

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <fcntl.h>
#include <math.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include "shapes96.h"

int _width = 0;
int _height = 0;

extern unsigned char ucFont[], ucSmallFont[];
static int iScreenOffset; // current write offset of screen data
static unsigned char ucScreen[1024]; // local copy of the image buffer
static int file_i2c = 0;
static int oled_type, oled_flip;

static void oledWriteCommand(unsigned char);
//
// Opens a file system handle to the I2C device
// Initializes the OLED controller into "page mode"
// Prepares the font data for the orientation of the display
// Returns 0 for success, 1 for failure
//
int oledInit(int iChannel, int iAddr, int iType, int bFlip, int bInvert) {
	const unsigned char oled64_initbuf[]={
		0x00,0xae,0xa8,0x3f,0xd3,0x00,0x40,0xa1,0xc8,
		0xda,0x12,0x81,0xff,0xa4,0xa6,0xd5,0x80,0x8d,0x14,
		0xaf,0x20,0x02};
	const unsigned char oled32_initbuf[] = {
		0x00,0xae,0xd5,0x80,0xa8,0x1f,0xd3,0x00,0x40,0x8d,0x14,0xa1,0xc8,0xda,0x02,
		0x81,0x7f,0xd9,0xf1,0xdb,0x40,0xa4,0xa6,0xaf};

	char filename[32];
	unsigned char uc[4];

	oled_type = iType;
	oled_flip = bFlip;
	sprintf(filename, "/dev/i2c-%d", iChannel);
	if ((file_i2c = open(filename, O_RDWR)) < 0) {
		fprintf(stderr, "Failed to open i2c bus %d\n", iChannel);
		file_i2c = 0;
		return 1;
	}

	if (ioctl(file_i2c, I2C_SLAVE, iAddr) < 0) {
		fprintf(stderr, "Failed to acquire bus access or talk to slave\n");
		file_i2c = 0;
		return 1;
	}

	if (iType == OLED_128x32) {
		_width = 128;
		_height = 32;
		write(file_i2c, oled32_initbuf, sizeof(oled32_initbuf));
	} else {
		_width = 128;
		_height = 64;
		write(file_i2c, oled64_initbuf, sizeof(oled64_initbuf));
	}
	if (bInvert) {
		uc[0] = 0; // command
		uc[1] = 0xa7; // invert command
		write(file_i2c, uc, 2);
	}
	if (bFlip) {	// rotate display 180
		uc[0] = 0; // command
		uc[1] = 0xa0;
		write(file_i2c, uc, 2);
		uc[1] = 0xc0;
		write(file_i2c, uc, 2);
	}
	return 0;
}

// Sends a command to turn off the OLED display
// Closes the I2C file handle
void oledShutdown() {
	if (file_i2c != 0) {
		oledWriteCommand(0xaE); // turn off OLED
		close(file_i2c);
		file_i2c = 0;
	}
}

// Send a single byte command to the OLED controller
static void oledWriteCommand(unsigned char c) {
	unsigned char buf[2];
	int rc;

	buf[0] = 0x00; // command introducer
	buf[1] = c;
	rc = write(file_i2c, buf, 2);
	if (rc) {} // suppress warning
}

static void oledWriteCommand2(unsigned char c, unsigned char d) {
	unsigned char buf[3];
	int rc;

	buf[0] = 0x00;
	buf[1] = c;
	buf[2] = d;
	rc = write(file_i2c, buf, 3);
	if (rc) {} // suppress warning
}

int oledSetContrast(unsigned char ucContrast) {
        if (file_i2c == 0)
                return -1;

	oledWriteCommand2(0x81, ucContrast);
	return 0;
}

// Send commands to position the "cursor" to the given
// row and column
static void oledSetPosition(int x, int y) {
	iScreenOffset = (y*128)+x;
	if (oled_type == OLED_64x32) {	// visible display starts at column 32, row 4
		x += 32; // display is centered in VRAM, so this is always true
		if (oled_flip == 0) // non-flipped display starts from line 4
		y += 4;
	} else if (oled_type == OLED_132x64) {	// SH1106 has 128 pixels centered in 132
		x += 2;
	}

	oledWriteCommand(0xb0 | y); // go to page Y
	oledWriteCommand(0x00 | (x & 0xf)); // // lower col addr
	oledWriteCommand(0x10 | ((x >> 4) & 0xf)); // upper col addr
}

// Write a block of pixel data to the OLED
// Length can be anything from 1 to 1024 (whole display)
static void oledWriteDataBlock(unsigned char *ucBuf, int iLen) {
	unsigned char ucTemp[129];
	int rc;

	ucTemp[0] = 0x40; // data command
	memcpy(&ucTemp[1], ucBuf, iLen);
	rc = write(file_i2c, ucTemp, iLen+1);
	if (rc) {} // suppress warning
	// Keep a copy in local buffer
	memcpy(&ucScreen[iScreenOffset], ucBuf, iLen);
	iScreenOffset += iLen;
}

// Set (or clear) an individual pixel
// The local copy of the frame buffer is used to avoid
// reading data from the display controller
int oledSetPixel(int x, int y, unsigned char ucColor) {
	int i;
	unsigned char uc, ucOld;

	if (file_i2c == 0)
		return -1;

	i = ((y >> 3) * 128) + x;
	if (i < 0 || i > 1023) // off the screen
		return -1;
	uc = ucOld = ucScreen[i];
	uc &= ~(0x1 << (y & 7));
	if (ucColor) {
		uc |= (0x1 << (y & 7));
	}
	if (uc != ucOld) {	// pixel changed
		oledSetPosition(x, y>>3);
		oledWriteDataBlock(&uc, 1);
	}
	return 0;
}
//
// Draw a string of small (8x8), large (16x24), or very small (6x8)  characters
// At the given col+row
// The X position is in character widths (8 or 16)
// The Y position is in memory pages (8 lines each)
//
int oledWriteString(int x, int y, char *szMsg, int iSize) {
	int i, iLen;
	unsigned char *s;

	if (file_i2c == 0)
		return -1; // not initialized
	if (iSize < FONT_NORMAL || iSize > FONT_SMALL)
		return -1;

	iLen = strlen(szMsg);
	if (iSize == FONT_BIG) {	// draw 16x32 font
		if (iLen+x > 8) iLen = 8-x;
		if (iLen < 0) return -1;
		x *= 16;
		for (i=0; i<iLen; i++) {
			s = &ucFont[9728 + (unsigned char)szMsg[i]*64];
			oledSetPosition(x+(i*16), y);
			oledWriteDataBlock(s, 16);
			oledSetPosition(x+(i*16), y+1);
			oledWriteDataBlock(s+16, 16);	
			oledSetPosition(x+(i*16), y+2);
			oledWriteDataBlock(s+32, 16);	
//			oledSetPosition(x+(i*16), y+3);
//			oledWriteDataBlock(s+48, 16);	
		}
	} else if (iSize == FONT_NORMAL) {	// draw 8x8 font
		oledSetPosition(x*8, y);
		if (iLen + x > 16) iLen = 16 - x; // can't display it
		if (iLen < 0)return -1;

		for (i=0; i<iLen; i++)
		{
			s = &ucFont[(unsigned char)szMsg[i] * 8];
			oledWriteDataBlock(s, 8); // write character pattern
		}	
	} else {	// 6x8
		oledSetPosition(x*6, y);
		if (iLen + x > 21) iLen = 21 - x;
		if (iLen < 0) return -1;
		for (i=0; i<iLen; i++)
		{
			s = &ucSmallFont[(unsigned char)szMsg[i]*6];
			oledWriteDataBlock(s, 6);
		}
	}
	return 0;
}

// Fill the frame buffer with a byte pattern
// e.g. all off (0x00) or all on (0xff)
int oledFill(unsigned char ucData) {
	int y;
	unsigned char temp[128];
	int iLines, iCols;

	if (file_i2c == 0)
		return -1; // not initialized

	iLines = (oled_type == OLED_128x32 || oled_type == OLED_64x32) ? 4:8;
	iCols = (oled_type == OLED_64x32) ? 4:8;

	memset(temp, ucData, 128);
	for (y=0; y<iLines; y++) {
		oledSetPosition(0,y); // set to (0,Y)
		oledWriteDataBlock(temp, iCols*16); // fill with data byte
	} // for y
	return 0;
} /* oledFill() */

// Draw a line of text using varaible arguments like printf().
int oledPrintf(int x, int y, char *szMsg, int iSize, ...) {
	va_list args;
	char buf[2048];

	va_start(args, iSize);

	int r = vsprintf(buf, szMsg, args);

	oledWriteString(x, y, buf, iSize);

	va_end(args);

	return r;	
}

// Draw a circle.
int oledCircle(int xc, int yc, int r, unsigned char color) {

	int x = 0;
    int y = r;
    int d = 3 - 2 * r;

    while (y >= x)
    {
        oledSetPixel(xc + x, yc + y, color);
        oledSetPixel(xc - x, yc + y, color);
        oledSetPixel(xc + x, yc - y, color);
        oledSetPixel(xc - x, yc - y, color);
        oledSetPixel(xc + y, yc + x, color);
        oledSetPixel(xc - y, yc + x, color);
        oledSetPixel(xc + y, yc - x, color);
        oledSetPixel(xc - y, yc - x, color);

        x++;

        if (d > 0) {
            y--;
            d = d + 4 * (x - y) + 10;
        } else {
            d = d + 4 * x + 6;
        }
    }

	return 0;
}

// -----------------------------------------------------------
// Draw a horizontal line
// -----------------------------------------------------------
void oledHLine(int x, int y, int length, unsigned char color)
{
    for (int i = 0; i < length; i++)
        oledSetPixel(x + i, y, color);
}

// -----------------------------------------------------------
// Draw a vertical line
// -----------------------------------------------------------
void oledVLine(int x, int y, int length, unsigned char color)
{
    for (int i = 0; i < length; i++)
        oledSetPixel(x, y + i, color);
}

int oledFilledCircle(int xc, int yc, int r, unsigned char color) {

	for (int y = yc - r; y <= yc + r; y++) {
        for (int x = xc - r; x <= xc + r; x++) {
            int dx = x - xc;
            int dy = y - yc;
            if (dx*dx + dy*dy <= r*r)
                oledSetPixel(x, y, color);
        }
    }

	return 0;
}

// Draw square.
int oledSquare(int x, int y, int size, unsigned char color) {

	oledHLine(x, y, size, color);              // top
    oledHLine(x, y + size - 1, size, color);   // bottom
    oledVLine(x, y, size, color);              // left
    oledVLine(x + size - 1, y, size, color);   // right
	
	return 0;
}

int oledFilledSquare(int x, int y, int size, unsigned char color) {

	for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            oledSetPixel(x + i, y + j, color);
        }
    }

	return 0;
}

int oledRectangle(int x, int y, int width, int height, unsigned char color) {

	oledHLine(x, y, width, color);             // top
    oledHLine(x, y + height - 1, width, color); // bottom
    oledVLine(x, y, height, color);           // left
    oledVLine(x + width - 1, y, height, color); // right
	
	return 0;
}

int oledFilledRectangle(int x, int y, int width, int height, unsigned char color) {
	for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            oledSetPixel(x + i, y + j, color);
        }
    }

	return 0;
}

// Draw Ellipse.
int oledEllipse(int xc, int yc, int rx, int ry, unsigned char color) {

	int x = 0;
    int y = ry;

    // Initial decision parameter of region 1
    long rx2 = (long)rx * rx;
    long ry2 = (long)ry * ry;
    long two_rx2 = 2 * rx2;
    long two_ry2 = 2 * ry2;
    long px = 0;
    long py = two_rx2 * y;

    // Region 1
    long p = (long)(ry2 - (rx2 * ry) + (0.25 * rx2));
    while (px < py) {
        oledSetPixel(xc + x, yc + y, color);
        oledSetPixel(xc - x, yc + y, color);
        oledSetPixel(xc + x, yc - y, color);
        oledSetPixel(xc - x, yc - y, color);

        x++;
        px += two_ry2;
        if (p < 0) {
            p += ry2 + px;
        } else {
            y--;
            py -= two_rx2;
            p += ry2 + px - py;
        }
    }

    // Region 2
    p = (long)(ry2 * (x + 0.5) * (x + 0.5) + rx2 * (y - 1) * (y - 1) - rx2 * ry2);
    while (y >= 0) {
        oledSetPixel(xc + x, yc + y, color);
        oledSetPixel(xc - x, yc + y, color);
        oledSetPixel(xc + x, yc - y, color);
        oledSetPixel(xc - x, yc - y, color);

        y--;
        py -= two_rx2;
        if (p > 0) {
            p += rx2 - py;
        } else {
            x++;
            px += two_ry2;
            p += rx2 - py + px;
        }
    }

	return 0;
}

int oledFilledEllipse(int xc, int yc, int rx, int ry, unsigned char color) {

	for (int y = yc - ry; y <= yc + ry; y++) {
        for (int x = xc - rx; x <= xc + rx; x++) {
            int dx = x - xc;
            int dy = y - yc;
            // Check ellipse equation
            if ((dx*dx)*ry*ry + (dy*dy)*rx*rx <= rx*rx*ry*ry)
                oledSetPixel(x, y, color);
        }
    }

	return 0;
}

// Draw a line.
int oledLine(int x0, int y0, int x1, int y1, unsigned char color) {

	int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (1) {
        oledSetPixel(x0, y0, color);
        if (x0 == x1 && y0 == y1)
            break;
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }

	return 0;
}

// -----------------------------------------------------------
// Helper: Swap integers
// -----------------------------------------------------------
void swap_int(int* a, int* b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

// Draw a triangle.
int oledTriangle(int x0, int y0, int x1, int y1, int x2, int y2, unsigned char color) {

	oledLine(x0, y0, x1, y1, color);
    oledLine(x1, y1, x2, y2, color);
    oledLine(x2, y2, x0, y0, color);
	
	return 0;
}

int oledFilledTriangle(int x0, int y0, int x1, int y1, int x2, int y2, unsigned char color) {

	// Sort vertices by y-coordinate ascending (y0 <= y1 <= y2)
    if (y0 > y1) { swap_int(&y0, &y1); swap_int(&x0, &x1); }
    if (y1 > y2) { swap_int(&y1, &y2); swap_int(&x1, &x2); }
    if (y0 > y1) { swap_int(&y0, &y1); swap_int(&x0, &x1); }

    int total_height = y2 - y0;

    for (int i = 0; i <= total_height; i++) {
        int second_half = i > (y1 - y0) || y1 == y0;
        int segment_height = second_half ? y2 - y1 : y1 - y0;
        float alpha = (float)i / total_height;
        float beta = (float)(i - (second_half ? y1 - y0 : 0)) / segment_height;

        int ax = x0 + (x2 - x0) * alpha;
        int ay = y0 + i;
        int bx = second_half ? x1 + (x2 - x1) * beta : x0 + (x1 - x0) * beta;

        if (ax > bx) swap_int(&ax, &bx);

        for (int x = ax; x <= bx; x++)
            oledSetPixel(x, ay, color);
    }

	return 0;
}

// Draw a polygon.
int oledPolygon(int *vx, int *vy, int vertices, unsigned char color) {

	for (int i = 0; i < vertices; i++) {
        int next = (i + 1) % vertices; // wrap last point to first
        oledLine(vx[i], vy[i], vx[next], vy[next], color);
    }
	
	return 0;
}
int oledFilledPolygon(int *vx, int *vy, int vertices, unsigned char color) {

	// Find min and max Y
    int minY = vy[0], maxY = vy[0];
    for (int i = 1; i < vertices; i++) {
        if (vy[i] < minY) minY = vy[i];
        if (vy[i] > maxY) maxY = vy[i];
    }

    // Scan-line from minY to maxY
    for (int y = minY; y <= maxY; y++) {
        int nodes = 0;
        int nodeX[_width]; // store intersections

        // Find intersections with polygon edges
        for (int i = 0; i < vertices; i++) {
            int j = (i + 1) % vertices;
            int yi = vy[i], yj = vy[j];
            int xi = vx[i], xj = vx[j];

            if ((yi < y && yj >= y) || (yj < y && yi >= y)) {
                int x = xi + (y - yi) * (xj - xi) / (yj - yi);
                nodeX[nodes++] = x;
            }
        }

        // Sort the nodes
        for (int i = 0; i < nodes-1; i++) {
            for (int j = i+1; j < nodes; j++) {
                if (nodeX[i] > nodeX[j]) swap_int(&nodeX[i], &nodeX[j]);
            }
        }

        // Fill pixels between node pairs
        for (int i = 0; i < nodes; i += 2) {
            if (i+1 >= nodes) break;
            for (int x = nodeX[i]; x <= nodeX[i+1]; x++)
                oledSetPixel(x, y, color);
        }
    }
	
	return 0;
}

int oledArc(int xc, int yc, int r, float sa, float ea, unsigned char color) {

	// Convert degrees to radians
    float start_rad = sa * M_PI / 180.0f;
    float end_rad = ea * M_PI / 180.0f;

    float step = 0.01f; // smaller = smoother arc

    for (float theta = start_rad; theta <= end_rad; theta += step)
    {
        int x = xc + (int)(r * cosf(theta));
        int y = yc + (int)(r * sinf(theta));
        oledSetPixel(x, y, color);
    }
	
	return 0;
}

int oledBezier(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3, unsigned char color) {

	int prev_x = x0;
    int prev_y = y0;
    float step = 0.01f; // smaller = smoother curve

    for (float t = step; t <= 1.0f; t += step)
    {
        float u = 1 - t;
        float tt = t * t;
        float uu = u * u;
        float uuu = uu * u;
        float ttt = tt * t;

        float xf = uuu * x0
                 + 3 * uu * t * x1
                 + 3 * u * tt * x2
                 + ttt * x3;

        float yf = uuu * y0
                 + 3 * uu * t * y1
                 + 3 * u * tt * y2
                 + ttt * y3;

        int xi = (int)(xf + 0.5f);
        int yi = (int)(yf + 0.5f);

        oledLine(prev_x, prev_y, xi, yi, color);
        prev_x = xi;
        prev_y = yi;
    }

	return 0;
}

int oledParabola(int h, int k, float a, int xs, int xe, unsigned char color) {

	int prev_x = xs;
    int prev_y = k + (int)(a * (xs - h) * (xs - h) + 0.5f);

    for (int x = xs + 1; x <= xe; x++)
    {
        int y = k + (int)(a * (x - h) * (x - h) + 0.5f);
        // oledSetPixel(x, y, color);
        // Optional: draw line for smoother curve
        oledLine(prev_x, prev_y, x, y, color);
        prev_x = x;
        prev_y = y;
    }
	
	return 0;
}
