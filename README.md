# shapes96
This code comes from Larry Banks, oled_96. If you want to read his orignal README.md then see README.md.old.

This is simple C library for working with SSD1306/SH1106 OLED displays when connected to the I2C bus.

Larry Banks had deprecated this library but I resurrected it because I could not get the other to compile and
wanted a simple C library to use.

I have enhanced the library to add the ability to draw shapes.

This has 3 font sizes:
    FONT_NORMAL     // 8x8
    FONT_SMALL      // 6x8
    FONT_BIG        // 16x24

This suppports 4 screen resolutions:
    OLED_128x32
    OLED_128x64
    OLED_132x64
    OLED_64x32

Some of the shapes are:
    Circle
    Square
    Retangle
    Line
    Arc
    Bezier
    Parabola
    Triangle
    Ellipse
    Ploygon

I changed all of the funciton calls to reflex my sytle of coding.
I also enhanced the sample.c program to show the shapes.
I also combined the makefile and make_sample together.

To build:
    cd shapes96
    make

Kelly Wiles
rkwiles@twc.com
