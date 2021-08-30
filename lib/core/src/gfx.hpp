#include "Adafruit_GFX.h"
#include "Adafruit_RA8875.h"

#define RA8875_CS 10
#define RA8875_RESET 9

#define RA8875_LIGHT_GREY 0x8C71
#define RA8875_DARK_GREY 0x2965

#define RA8875_TEXT_SM 0
#define RA8875_TEXT_MD 1
#define RA8875_TEXT_LG 2

#define DEG2RAD 0.0174532925
#define BYTE2DEG 1.41176470588

#define LAYOUT_DIAL_RADIUS 45
#define LAYOUT_DIAL_Y 50
#define LAYOUT_ROW_LINE_Y 150
#define LAYOUT_PARAM_Y 167

#define CHANNEL_COUNT 8

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
  float rads = (value * BYTE2DEG + 180) * DEG2RAD; // convert value to radians
  int xEnd = xStart + length * sin(rads); // Ending x-coordinate offset & radius
  int yEnd = yStart - length * cos(rads); // Ending y-coordinate offset & radius
  gfx_tft.drawLine(xStart, yStart, xEnd, yEnd, color);
}

void gfx_drawDial(byte row, byte channel, bool isBipolar, byte oldValue, byte newValue) {
  unsigned int x = gfx_getDialX(channel);
  unsigned int y = gfx_getDialY(row);

  if (newValue != oldValue) {
    // Clear existing line
    gfx_drawValueLine(x, y, oldValue, LAYOUT_DIAL_RADIUS - 10, RA8875_BLACK);
  };
  
  // Draw value text
  char buffer[5];
  if (isBipolar) {
    int dispVal = newValue / 2 - 64;
    char sign = dispVal < 0 ? '-' : '+';
    sprintf(buffer, "%c%02d", sign, abs(dispVal));
  } else {
    sprintf(buffer, "%03d", newValue / 2);
  }
  gfx_drawText(x - 22, y - 18, RA8875_TEXT_MD, RA8875_WHITE, buffer);

  // Draw value line
  gfx_drawValueLine(x, y, newValue, LAYOUT_DIAL_RADIUS - 10, RA8875_RED);
}

void gfx_drawParamName(byte row, const char* name) {
  gfx_drawText(8, row == 0 ? LAYOUT_PARAM_Y : 426 - LAYOUT_PARAM_Y, RA8875_TEXT_LG, RA8875_LIGHT_GREY, name);
}

void gfx_drawRow(byte row, const char* paramName, bool isBipolar, const byte* oldValues, const byte* newValues) {
  unsigned int x;
  unsigned int y = gfx_getDialY(row);
  unsigned int yTriB = y + LAYOUT_DIAL_RADIUS;
  unsigned int yTriT = y - LAYOUT_DIAL_RADIUS;
  for (byte channel = 0; channel < CHANNEL_COUNT; channel++) {
    gfx_drawDial(row, channel, isBipolar, oldValues[channel], newValues[channel]);
    x = gfx_getDialX(channel);
    gfx_tft.drawTriangle(x - 5, yTriB, x + 5, yTriB, x, yTriB - 10, RA8875_RED);
    gfx_tft.drawTriangle(x - 5, yTriT, x + 5, yTriT, x, yTriT + 10, RA8875_RED);
  }

  gfx_drawParamName(row, paramName);
}

void gfx_drawChannelNumbers() {
  int x;
  int y;
  char buffer [2];
  for (byte row = 0; row < 2; row++) {
    y = gfx_getDialY(row);
    y = row == 0 ? y + LAYOUT_DIAL_RADIUS + 3 : y - LAYOUT_DIAL_RADIUS - 35;
    for (byte channel = 0; channel < CHANNEL_COUNT; channel++) {
      x = gfx_getDialX(channel);
      itoa(channel + 1, buffer, 10);
      gfx_drawText(x - 10, y, RA8875_TEXT_MD, RA8875_LIGHT_GREY, buffer);
    }
  }
}

void gfx_drawBackground(void) {
  gfx_tft.drawFastHLine(0, LAYOUT_ROW_LINE_Y, 800, RA8875_LIGHT_GREY);
  gfx_tft.drawFastHLine(0, 480 - LAYOUT_ROW_LINE_Y, 800, RA8875_LIGHT_GREY);
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

  gfx_drawBackground();
  gfx_drawChannelNumbers();
}