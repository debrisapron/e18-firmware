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

#define LAYOUT_KNOB_RADIUS 45
#define LAYOUT_KNOB_Y 50
#define LAYOUT_ROW_LINE_Y 150
#define LAYOUT_PARAM_Y 167

Adafruit_RA8875 gfx_tft = Adafruit_RA8875(RA8875_CS, RA8875_RESET);

// #############################################################################
// Draw a circular or elliptical arc with a defined thickness
// #############################################################################

// x,y == coords of centre of arc
// start_angle = 0 - 359
// seg_count = number of 3 degree segments to draw (120 => 360 degree arc)
// rx = x axis radius
// yx = y axis radius
// w  = width (thickness) of arc in pixels
// colour = 16 bit colour value
// Note if rx and ry are the same then an arc of a circle is drawn

void gfx_fillArc2(unsigned int x, unsigned int y, int start_angle, int seg_count, int rx, int ry, int w, unsigned int colour)
{
  byte seg = 3; // Segments are 3 degrees wide = 120 segments for 360 degrees
  byte inc = 3; // Draw segments every 3 degrees, increase to 6 for segmented ring

  // Calculate first pair of coordinates for segment start
  float sx = cos((start_angle - 90) * DEG2RAD);
  float sy = sin((start_angle - 90) * DEG2RAD);
  uint16_t x0 = sx * (rx - w) + x;
  uint16_t y0 = sy * (ry - w) + y;
  uint16_t x1 = sx * rx + x;
  uint16_t y1 = sy * ry + y;

  // Draw colour blocks every inc degrees
  for (int i = start_angle; i < start_angle + seg * seg_count; i += inc) {

    // Calculate pair of coordinates for segment end
    float sx2 = cos((i + seg - 90) * DEG2RAD);
    float sy2 = sin((i + seg - 90) * DEG2RAD);
    int x2 = sx2 * (rx - w) + x;
    int y2 = sy2 * (ry - w) + y;
    int x3 = sx2 * rx + x;
    int y3 = sy2 * ry + y;

    gfx_tft.fillTriangle(x0, y0, x1, y1, x2, y2, colour);
    gfx_tft.fillTriangle(x1, y1, x2, y2, x3, y3, colour);

    // Copy segment end to segment start for next segment
    x0 = x2;
    y0 = y2;
    x1 = x3;
    y1 = y3;
  }
}

unsigned int gfx_getKnobX(byte channel) {
  return (channel * 100) + 50;
}

unsigned int gfx_getKnobY(byte row) {
  return row == 0 ? LAYOUT_KNOB_Y : 480 - LAYOUT_KNOB_Y;
}

void gfx_drawText(unsigned int x, unsigned int y, byte size, unsigned int color, const char* buffer) {
  gfx_tft.textMode();
  gfx_tft.textSetCursor(x, y);
  gfx_tft.textColor(color, RA8875_BLACK);
  gfx_tft.textEnlarge(size);
  gfx_tft.textWrite(buffer);
  gfx_tft.graphicsMode();
}

void gfx_drawKnob(byte row, byte channel, bool isBipolar, int currValue, int newValue) {
  int start, segments;
  unsigned int x = gfx_getKnobX(channel);
  unsigned int y = gfx_getKnobY(row);
  unsigned int color;

  // Draw value arc
  if (newValue != currValue) {
    if ((newValue >= 0 && newValue < currValue) || (newValue <= 0 && (newValue > currValue))) {
      // Remove segments
      start = newValue * 3;
      if (!isBipolar) {
        start += 180;
      }
      segments = abs(currValue - newValue);
      color = RA8875_DARK_GREY;
    } else {
      // Add segments
      start = currValue * 3;
      if (!isBipolar) {
        start += 180;
      }
      segments = abs(newValue - currValue);
      color = RA8875_WHITE;
    }
    gfx_fillArc2(x, y, start, segments, LAYOUT_KNOB_RADIUS, LAYOUT_KNOB_RADIUS, 10, color);
  }

  // Draw value text
  char buffer [4];
  sprintf(buffer, "%3i", newValue);
  gfx_drawText(x - 23, y - 20, RA8875_TEXT_MD, RA8875_WHITE, buffer);
}

void gfx_drawParamName(byte row, const char* name) {
  gfx_drawText(8, row == 0 ? LAYOUT_PARAM_Y : 426 - LAYOUT_PARAM_Y, RA8875_TEXT_LG, RA8875_LIGHT_GREY, name);
}

void gfx_drawKnobBackground(byte row, byte channel) {
  // Draw thick grey circle
  unsigned int x = gfx_getKnobX(channel);
  unsigned int y = gfx_getKnobY(row);
  gfx_tft.fillCircle(x, y, LAYOUT_KNOB_RADIUS, RA8875_DARK_GREY);
  gfx_tft.fillCircle(x, y, LAYOUT_KNOB_RADIUS - 10, RA8875_BLACK);

  // Draw channel number
  char buffer [2];
  itoa(channel + 1, buffer, 10);
  y = row == 0 ? y + LAYOUT_KNOB_RADIUS + 3 : y - LAYOUT_KNOB_RADIUS - 35;
  gfx_drawText(x - 10, y, RA8875_TEXT_MD, RA8875_LIGHT_GREY, buffer);
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

  // Draw lines
  gfx_drawBackground();
}