#include <Adafruit_GFX.h>
#include <Adafruit_RA8875.h>
#include "shared.h"

#define RA8875_CS 10
#define RA8875_RESET 9

#define RA8875_LIGHT_GREY 0x8C71
#define RA8875_DARK_GREY 0x2965

#define RA8875_TEXT_SM 0
#define RA8875_TEXT_MD 1
#define RA8875_TEXT_LG 2

#define BYTE_TO_DEG 1.41176470588

#define LAYOUT_DIAL_RADIUS 45
#define LAYOUT_DIAL_Y 50
#define LAYOUT_ROW_LINE_Y 150
#define LAYOUT_PARAM_Y 167

Adafruit_RA8875 gfx_tft = Adafruit_RA8875(RA8875_CS, RA8875_RESET);

// extern void __log(const char* name, const char* value);

unsigned int gfx_getDialX(byte channel) {
  return (channel * 100) + 50;
}

unsigned int gfx_getDialY(byte row) {
  return row == 0 ? LAYOUT_DIAL_Y : 480 - LAYOUT_DIAL_Y;
}

void gfx_drawText(unsigned int x, unsigned int y, byte size, unsigned int color, const char* buffer) {
  gfx_tft.textMode();
  gfx_tft.textSetCursor(x, y);
  gfx_tft.textColor(color, RA8875_BLACK);
  gfx_tft.textEnlarge(size);
  gfx_tft.textWrite(buffer);
  gfx_tft.graphicsMode();
}

// value param is 0-255 mapped to 180-540 degrees
void gfx_drawValueLine(int xStart, int yStart, byte value, int length, int color) {
  float rads = (value * BYTE_TO_DEG + 180) * DEG_TO_RAD; // convert value to radians
  int xEnd = xStart + length * sin(rads); // Ending x-coordinate offset & radius
  int yEnd = yStart - length * cos(rads); // Ending y-coordinate offset & radius
  gfx_tft.drawLine(xStart, yStart, xEnd, yEnd, color);
}

void gfx_drawDial(byte row, byte channel, bool isScalar, byte oldValue, byte newValue, const char* displayValue) {
  unsigned int x = gfx_getDialX(channel);
  unsigned int y = gfx_getDialY(row);

  // Clear existing line
  gfx_drawValueLine(x, y, oldValue, LAYOUT_DIAL_RADIUS - 10, RA8875_BLACK);
  
  // Print display value
  gfx_drawText(x - 22, y - 18, RA8875_TEXT_MD, RA8875_WHITE, displayValue);

  if (isScalar) {
    // Draw value line
    gfx_drawValueLine(x, y, newValue, LAYOUT_DIAL_RADIUS - 10, RA8875_RED);
  }
}

void gfx_drawParamName(byte row, const char* name) {
  // Right-pad name to fully overwrite previous name
  char buffer[11];
  sprintf(buffer, "%-10s", name);
  gfx_drawText(8, row == 0 ? LAYOUT_PARAM_Y : 426 - LAYOUT_PARAM_Y, RA8875_TEXT_LG, RA8875_LIGHT_GREY, buffer);
}

void gfx_drawStaticElements(void) {
  unsigned int x;
  unsigned int y;
  unsigned int yChanNo;
  unsigned int yTriB;
  unsigned int yTriT;
  char buffer [2];

  // Draw horizontal dividers
  gfx_tft.drawFastHLine(0, LAYOUT_ROW_LINE_Y, 800, RA8875_LIGHT_GREY);
  gfx_tft.drawFastHLine(0, 480 - LAYOUT_ROW_LINE_Y, 800, RA8875_LIGHT_GREY);

  // Draw row static elements
  for (byte row = 0; row < 2; row++) {
    y = gfx_getDialY(row);
    yChanNo = row == 0 ? y + LAYOUT_DIAL_RADIUS + 3 : y - LAYOUT_DIAL_RADIUS - 35;
    yTriB = y + LAYOUT_DIAL_RADIUS;
    yTriT = y - LAYOUT_DIAL_RADIUS;
    for (byte channel = 0; channel < CHANNEL_COUNT; channel++) {
      x = gfx_getDialX(channel);

      // Draw channel number
      itoa(channel + 1, buffer, 10);
      gfx_drawText(x - 10, yChanNo, RA8875_TEXT_MD, RA8875_LIGHT_GREY, buffer);

      // Draw dial arrows
      gfx_tft.drawTriangle(x - 5, yTriB, x + 5, yTriB, x, yTriB - 10, RA8875_RED);
      gfx_tft.drawTriangle(x - 5, yTriT, x + 5, yTriT, x, yTriT + 10, RA8875_RED);
    }
  }
}

void gfx_start(void) {
  gfx_tft.fillScreen(RA8875_BLACK);
  gfx_drawStaticElements();
}

void gfx_setup(void) {
  // Start TFT
  bool ok = gfx_tft.begin(RA8875_800x480);
  if (!ok) {
    Serial.println("RA8875 Not Found!");
    return;
  }
  gfx_tft.displayOn(true);
  gfx_tft.GPIOX(true); // Enable TFT - display enable tied to GPIOX
  gfx_tft.PWM1config(true, RA8875_PWM_CLK_DIV1024); // PWM output for backlight
  gfx_tft.PWM1out(255);

  gfx_drawText(100, 260, RA8875_TEXT_LG, RA8875_WHITE, "Initializing...");
}