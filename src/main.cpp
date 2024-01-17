/*

  ScrollingText.ino

  This will scroll text on the display.
  Enable U8g2 16 bit mode (see FAQ) for larger text!

  Universal 8bit Graphics Library (https://github.com/olikraus/u8g2/)

  Copyright (c) 2016, olikraus@gmail.com
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

  * Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
  CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include <Arduino.h>
#include <U8g2lib.h>

#define U8X8_HAVE_HW_I2C 1
#undef U8X8_HAVE_HW_SPI
#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

/*
  U82glib Example Overview:
    Frame Buffer Examples: clearBuffer/sendBuffer. Fast, but may not work with
  all Arduino boards because of RAM consumption Page Buffer Examples:
  firstPage/nextPage. Less RAM usage, should work with all Arduino boards. U8x8
  Text Only Example: No RAM usage, direct communication with display controller.
  No graphics, 8x8 Text only.

  This is a page buffer example.

*/

// Please UNCOMMENT one of the contructor lines below
// U8g2 Contructor List (Picture Loop Page Buffer)
// The complete list is available here:
// https://github.com/olikraus/u8g2/wiki/u8g2setupcpp Please update the pin
// numbers according to your setup. Use U8X8_PIN_NONE if the reset pin is not
// connected
U8G2_SSD1327_EA_W128128_1_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE);
// End of constructor list

// This example shows a scrolling text.
// If U8G2_16BIT is not set (default), then the pixel width of the text must be
// lesser than 128 If U8G2_16BIT is set, then the pixel width an be up to 32000

u8g2_uint_t offset;   // current offset for the scrolling text
u8g2_uint_t width;    // pixel width of the scrolling text (must be lesser than
                      // 128 unless U8G2_16BIT is defined
char text[16] = "0";  // scroll this text from right to left
int n = 0;
int i = 1;
int dispWidth, dispHeight;

void setup(void) {
    Wire.setPins(21, 22);
    u8g2.begin();
    dispWidth = u8g2.getDisplayWidth();
    dispHeight = u8g2.getDisplayHeight();

    u8g2.setFont(u8g2_font_inb30_mr);  // set the target font to calculate the
                                       // pixel width
    u8g2.setBusClock(400000);

    u8g2.setFontMode(0);  // enable transparent mode, which is faster
}

void loop(void) {
    u8g2_uint_t x;
    u8g2.setFont(u8g2_font_inb30_mr);  // set the target font

    sprintf(text, "%dm", n);
    int textWidth = u8g2.getUTF8Width(text);
    x = int((dispWidth - textWidth) / 2);

    u8g2.firstPage();

    do {
        u8g2.drawHLine(0, 5, dispWidth);
        u8g2.drawUTF8(x, 70, text);
    } while (u8g2.nextPage());
    n += i;
    if (n == 0) {
        i = 1;
    }
    if (n == 20) {
        i = -1;
    }
}
