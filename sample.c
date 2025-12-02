//
// oled test program
//
// Written by Larry Bank
// Copyright 2017 BitBank Software, Inc. All Rights Reserved.
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

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "shapes96.h"

int main(int argc, char *argv[]) {
	int i, iChannel;
	int iOLEDAddr = 0x3c; // typical address; it can also be 0x3d
	int iOLEDType = OLED_128x64; // Change this for your specific display
	int bFlip = 0, bInvert = 0;

	i = -1;
	iChannel = -1;
	while (i != 0 && iChannel < 2) {	// try I2C channel 0 through 2
		iChannel++;
		i=oledInit(iChannel, iOLEDAddr, iOLEDType, bFlip, bInvert);
	}
	if (i == 0) {
		printf("Successfully opened I2C bus %d\n", iChannel);

		oledFill(0); // fill with black

		oledWriteString(0,0,"OLED 96 Library!",FONT_NORMAL);
		oledWriteString(3,1,"BIG!",FONT_BIG);
		oledWriteString(0,1,"Small", FONT_SMALL);
		oledPrintf(0,5,"Who is %s %d!",FONT_NORMAL, "Kelly", 2);
		oledWriteString(3, 7, "Example Text", FONT_SMALL);

		printf("Press ENTER\n");
		getchar();
		oledFill(0);

		oledCircle(54, 28, 20, 1);
		oledCircle(64, 28, 19, 1);
		oledWriteString(3, 7, "Circle", FONT_SMALL);

		printf("Press ENTER\n");
		getchar();
		oledFill(0);

		oledFilledCircle(64, 28, 19, 1);
		oledWriteString(3, 7, "Filled Circle", FONT_SMALL);

		printf("Press ENTER\n");
		getchar();
		oledFill(0);

		oledSquare(32, 16, 20, 1);
		oledSquare(64, 32, 20, 1);
		oledWriteString(3, 7, "Square", FONT_SMALL);

		printf("Press ENTER\n");
		getchar();
		oledFill(0);

		oledFilledSquare(32, 16, 20, 1);
		oledFilledSquare(64, 32, 20, 1);
		oledWriteString(3, 7, "Filled Square", FONT_SMALL);

		printf("Press ENTER\n");
		getchar();
		oledFill(0);

		oledRectangle(32, 16, 10, 10, 1);
		oledRectangle(64, 32, 15, 10, 1);
		oledWriteString(3, 7, "Rectangle", FONT_SMALL);

		printf("Press ENTER\n");
		getchar();
		oledFill(0);

		oledFilledRectangle(32, 16, 10, 10, 1);
		oledFilledRectangle(64, 32, 15, 10, 1);
		oledWriteString(3, 7, "Filled Rectangle", FONT_SMALL);

		printf("Press ENTER\n");
		getchar();
		oledFill(0);

		oledEllipse(96, 32, 30, 15, 1);
		oledEllipse(96, 32, 15, 30, 1);
		oledWriteString(3, 7, "Ellipse", FONT_SMALL);

		printf("Press ENTER\n");
		getchar();
		oledFill(0);

		oledFilledEllipse(96, 32, 30, 15, 1);
		oledFilledEllipse(96, 32, 15, 30, 1);
		oledWriteString(3, 7, "Filled Ellipse", FONT_SMALL);

		printf("Press ENTER\n");
		getchar();
		oledFill(0);

		oledTriangle(64, 10, 20, 50, 108, 50, 1);
		oledWriteString(3, 7, "Triangle", FONT_SMALL);

		printf("Press ENTER\n");
		getchar();
		oledFill(0);

		oledFilledTriangle(64, 10, 20, 50, 108, 50, 1);
		oledWriteString(3, 7, "Filled Triangle", FONT_SMALL);

		printf("Press ENTER\n");
		getchar();
		oledFill(0);

		oledLine(20, 40, 80, 40, 1);
		oledLine(20, 43, 80, 43, 1);
		oledWriteString(3, 7, "Line", FONT_SMALL);

		printf("Press ENTER\n");
		getchar();
		oledFill(0);

		int vx[5] = {64, 90, 76, 52, 38};  // X coordinates
		int vy[5] = {10, 30, 55, 55, 30};
		oledPolygon(vx, vy, 5, 1);
		oledWriteString(3, 7, "Polygon", FONT_SMALL);

		printf("Press ENTER\n");
		getchar();
		oledFill(0);

		oledFilledPolygon(vx, vy, 5, 1);
		oledWriteString(3, 7, "Filled Polygon", FONT_SMALL);

		printf("Press ENTER\n");
		getchar();
		oledFill(0);

		oledArc(16, 8, 20, 0, 90, 1);
		oledWriteString(3, 7, "Arc", FONT_SMALL);

		printf("Press ENTER\n");
		getchar();
		oledFill(0);

		oledBezier(10, 50, 30, 10, 90, 10, 110, 50, 1);
		oledWriteString(3, 7, "Bezier", FONT_SMALL);

		printf("Press ENTER\n");
		getchar();
		oledFill(0);

		oledParabola(64, 20, 0.02f, 0, 127, 1);
		oledWriteString(3, 7, "Parabola", FONT_SMALL);

		printf("Press ENTER to quit\n");
		getchar();
		oledShutdown();
	} else {
		printf("Unable to initialize I2C bus 0-2, please check your connections and verify the device address by typing 'i2cdetect -y <channel>\n");
	}
   return 0;
}
