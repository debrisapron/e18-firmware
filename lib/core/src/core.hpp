#include "Arduino.h"
#include "Adafruit_GFX.h"
#include "Adafruit_RA8875.h"

// Library only supports hardware SPI at this time
// Connect SCLK to Mega Digital #52
// Connect MISO to Mega Digital #50
// Connect MOSI to Mega Digital #51
#define RA8875_INT 3
#define RA8875_CS 10
#define RA8875_RESET 9

#define RA8875_SILVER 0x8C71

#define DEG2RAD 0.0174532925
#define KNOB_RADIUS 45

#define STATUS_INIT 0
#define STATUS_READY 1

Adafruit_RA8875 tft = Adafruit_RA8875(RA8875_CS, RA8875_RESET);
byte status = STATUS_INIT;
int state = 0;

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

void fillArc2(int x, int y, int start_angle, int seg_count, int rx, int ry, int w, unsigned int colour)
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

    tft.fillTriangle(x0, y0, x1, y1, x2, y2, colour);
    tft.fillTriangle(x1, y1, x2, y2, x3, y3, colour);

    // Copy segment end to sgement start for next segment
    x0 = x2;
    y0 = y2;
    x1 = x3;
    y1 = y3;
  }
}

int getKnobX(byte knobNo) {
  byte knobCol = knobNo < 8 ? knobNo : knobNo - 8;
  return (knobCol * 100) + 50;
}

int getKnobY(byte knobNo) {
  return knobNo < 8 ? 50 : 430;
}

void drawText(int x, int y, int value) {
  char buffer [3];
  tft.textMode();
  tft.textSetCursor(x, y);
  tft.textColor(RA8875_WHITE, RA8875_BLACK);
  tft.textEnlarge(1);
  sprintf(buffer, "%3u", value);
  tft.textWrite(buffer);
  tft.graphicsMode();
}

void drawKnob(byte knobNo, int currValue, int newValue) {
  int start, segments;
  int x = getKnobX(knobNo);
  int y = getKnobY(knobNo);
  unsigned int color;

  // Draw value arc
  if (newValue != currValue) {
    if (newValue < currValue) {
      start = newValue * 3;
      segments = currValue - newValue;
      color = RA8875_BLACK;
    } else {
      start = currValue * 3;
      segments = newValue - currValue;
      color = RA8875_WHITE;
    }
    fillArc2(x, y, start, segments, KNOB_RADIUS, KNOB_RADIUS, 10, color);
  }

  // Draw value text
  drawText(x - 23, y - 20, newValue);

  // Draw knob circle (unless value is going up in which case it isn't needed)
  if (newValue <= currValue) {
    tft.drawCircle(x, y, KNOB_RADIUS - 4, RA8875_WHITE);
  }
}

void handleKnob(int code) {
  int oldValue = state;
  if (code == 101) {
    state++;
  }
  if (code == 102) {
    state--;
  }
  drawKnob(0, oldValue, state);
}

void start(void) {
  Serial.begin(115200);

  // Start TFT
  bool ok = tft.begin(RA8875_800x480);
  if (!ok) {
    Serial.println("RA8875 Not Found!");
    return;
  }
  tft.displayOn(true);
  tft.GPIOX(true); // Enable TFT - display enable tied to GPIOX
  tft.PWM1config(true, RA8875_PWM_CLK_DIV1024); // PWM output for backlight
  tft.PWM1out(255);

  // Draw knob circles
  for (byte b = 0; b < 8; b++) {
    drawKnob(b, 0, 0);
  }

  status = STATUS_READY;
}

// MIDI_CREATE_DEFAULT_INSTANCE();
  // MIDI.begin(1);
  // Serial.begin(115200);
  // Serial.print("Status: "); Serial.println(tft.readStatus(), HEX);

  // // Draw 16 grey circles
  // for (byte i = 0; i < 8; i++) {
  //   int x = (i * 100) + 50;
  //   tft.fillCircle(x, 50, 45, 0x8C71);
  //   tft.fillCircle(x, 430, 45, 0x8C71);
  //   // int seg_count = (i * 15) + 5;
  //   // fillArc2(x, 50, 0, seg_count, 45, 45, 10, RA8875_WHITE);
  //   // fillArc2(x, 430, 0, seg_count, 45, 45, 10, RA8875_WHITE);
  // }

  // for (int i = 50; i < 74; i++) 
  // {
  //   MIDI.sendNoteOn(i, 64, 1); 
  //   delay(75); 
  //   MIDI.sendNoteOff(i, 64, 1); 
  //   delay(75); 
  // }

  // // Delay == Bad! Set up an interrupt based timer instead 
  // // (or something like Blink Without Delay)
  // delay(2000); 

  // // Print message
