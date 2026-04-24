# shapes96

A C library for drawing text and shapes on SSD1306/SH1106 OLED displays via the I2C bus on Linux (e.g., Raspberry Pi). Originally authored by Larry Bank (BitBank Software), this fork was enhanced by Kelly Wiles to add a full suite of 2D drawing primitives.

---

## Features

- **Multiple display types** — supports 128×32, 128×64, 132×64, and 64×32 OLED panels
- **Three font sizes** — Normal (8×8), Small (6×8), and Big (16×24)
- **Rich shape library** — outlines and filled variants of all common primitives
- **Curve drawing** — arc, cubic Bézier, and parabola functions
- **Arbitrary polygons** — both outlined and scanline-filled
- **printf-style text output** — format strings directly to the display
- **Pure C, minimal dependencies** — only requires `libc` and `libm`

---

## Hardware Requirements

| Item | Details |
|---|---|
| Display controller | SSD1306 or SH1106 |
| Interface | I²C |
| Typical I²C address | `0x3C` (may also be `0x3D`) |
| Supported resolutions | 128×32, 128×64, 132×64, 64×32 |

### Increase I²C Speed (Optional)

For faster refresh rates, add the following line to `/boot/config.txt` and reboot:

```
dtparam=i2c_arm_baudrate=400000
```

---

## Building

```bash
git clone https://github.com/tigerkelly/shapes96.git
cd shapes96
make
```

This will:
1. Compile `shapes96.c` and `fonts.c` into object files
2. Archive them into `libshapes96.a`
3. Copy the static library to `/usr/local/lib`
4. Copy `shapes96.h` to `/usr/local/include`
5. Compile the `sample` demo program

To clean build artifacts:

```bash
make clean
```

---

## Usage

### Initialization

```c
#include "shapes96.h"

// Open I2C channel, set address, display type, flip, and invert
int ret = oledInit(1, 0x3C, OLED_128x64, 0, 0);
if (ret != 0) {
    fprintf(stderr, "Failed to initialize display\n");
}
```

### Shutdown

```c
oledShutdown();
```

---

## API Reference

### Display Control

| Function | Description |
|---|---|
| `oledInit(channel, address, type, flip, invert)` | Initialize the display. Returns `0` on success, `1` on failure. |
| `oledShutdown()` | Turn off the display and close the I²C handle. |
| `oledFill(pattern)` | Fill the entire display with a byte pattern (`0x00` = black, `0xFF` = white). |
| `oledSetContrast(contrast)` | Set brightness level (`0`–`255`). |

### Display Types

```c
OLED_128x32   // 128×32 pixel display
OLED_128x64   // 128×64 pixel display
OLED_132x64   // 132×64 pixel display (SH1106)
OLED_64x32    // 64×32 pixel display
```

### Text Output

```c
// Write a string at column x (0–127), row y (0–7)
int oledWriteString(int x, int y, char *text, int fontSize);

// printf-style formatted output
int oledPrintf(int x, int y, char *fmt, int fontSize, ...);
```

**Font sizes:**

```c
FONT_NORMAL   // 8×8 pixels
FONT_SMALL    // 6×8 pixels
FONT_BIG      // 16×24 pixels
```

**Example:**

```c
oledWriteString(0, 0, "Hello, World!", FONT_NORMAL);
oledPrintf(0, 5, "Count: %d", FONT_SMALL, 42);
```

### Pixel

```c
// Set or clear a single pixel at (x, y); ucColor: 1=on, 0=off
int oledSetPixel(int x, int y, unsigned char ucColor);
```

### Lines & Curves

```c
// Straight line from (x0,y0) to (x1,y1)
int oledLine(int x0, int y0, int x1, int y1, unsigned char color);

// Arc centered at (xc,yc) with radius r, from angle sa to ea (degrees)
int oledArc(int xc, int yc, int r, float sa, float ea, unsigned char color);

// Cubic Bézier curve through four control points
int oledBezier(int x0, int y0, int x1, int y1,
               int x2, int y2, int x3, int y3, unsigned char color);

// Parabola with vertex at (h,k), coefficient a, from x=xs to x=xe
int oledParabola(int h, int k, float a, int xs, int xe, unsigned char color);
```

### Shapes (Outline & Filled)

| Shape | Outline | Filled |
|---|---|---|
| Circle | `oledCircle(xc, yc, r, color)` | `oledFilledCircle(xc, yc, r, color)` |
| Square | `oledSquare(x, y, size, color)` | `oledFilledSquare(x, y, size, color)` |
| Rectangle | `oledRectangle(x, y, w, h, color)` | `oledFilledRectangle(x, y, w, h, color)` |
| Ellipse | `oledEllipse(xc, yc, rx, ry, color)` | `oledFilledEllipse(xc, yc, rx, ry, color)` |
| Triangle | `oledTriangle(x0,y0, x1,y1, x2,y2, color)` | `oledFilledTriangle(x0,y0, x1,y1, x2,y2, color)` |
| Polygon | `oledPolygon(vx, vy, vertices, color)` | `oledFilledPolygon(vx, vy, vertices, color)` |

**Polygon example:**

```c
int vx[5] = {64, 90, 76, 52, 38};  // X coordinates
int vy[5] = {10, 30, 55, 55, 30};  // Y coordinates
oledPolygon(vx, vy, 5, 1);         // outline
oledFilledPolygon(vx, vy, 5, 1);   // filled
```

---

## Complete Example

```c
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include "shapes96.h"

void handleSig(int sig) {
    oledShutdown();
    exit(0);
}

int main(void) {
    signal(SIGINT, handleSig);

    if (oledInit(1, 0x3C, OLED_128x64, 0, 0) != 0) {
        fprintf(stderr, "Display init failed\n");
        return 1;
    }

    oledFill(0);                                    // clear screen
    oledWriteString(0, 0, "shapes96", FONT_NORMAL); // title
    oledCircle(64, 32, 20, 1);                      // circle outline
    oledFilledTriangle(20, 60, 50, 10, 80, 60, 1);  // filled triangle
    oledBezier(0, 63, 42, 0, 85, 0, 127, 63, 1);    // Bézier curve

    sleep(5);
    oledShutdown();
    return 0;
}
```

Compile against the installed library:

```bash
gcc -o my_app my_app.c -lshapes96 -lm
```

---

## File Structure

```
shapes96/
├── shapes96.h      # Public API header
├── shapes96.c      # Core library: I2C driver, text, and all shape functions
├── fonts.c         # Font bitmaps (Normal 8×8, Small 6×8, Big 16×24)
├── sample.c        # Interactive demo showcasing every shape
├── makefile        # Build rules for library and sample
├── LICENSE         # Apache License 2.0
└── README.md       # This file
```

---

## License

Licensed under the [Apache License 2.0](LICENSE).

Original library by **Larry Bank** (BitBank Software, Inc.) — Copyright © 2017.  
Enhanced with shape drawing functions by **Kelly Wiles** — 2025.

---

## Credits

- **Larry Bank** — original OLED96 I2C display library
- **Kelly Wiles** (`rkwiles@twc.com`) — shapes extension, `oledPrintf`, combined makefile, and sample enhancements
