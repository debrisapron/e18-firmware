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

#define STATUS_INIT 0
#define STATUS_READY 1

#define ACTION_NONE 0
#define ACTION_INC 1
#define ACTION_DEC 2

#define PARAM_VOL 0
#define PARAM_PAN 1
#define PARAM_COUNT 2
#define CHANNEL_COUNT 8

Adafruit_RA8875 tft = Adafruit_RA8875(RA8875_CS, RA8875_RESET);
byte status = STATUS_INIT;
byte param[] = {PARAM_VOL, PARAM_PAN};
int state[][CHANNEL_COUNT] = {
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0}
};
const char *paramNames[] = {"VOL", "PAN"};

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

void fillArc2(unsigned int x, unsigned int y, int start_angle, int seg_count, int rx, int ry, int w, unsigned int colour)
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

    // Copy segment end to segment start for next segment
    x0 = x2;
    y0 = y2;
    x1 = x3;
    y1 = y3;
  }
}

unsigned int getKnobX(byte channel) {
  return (channel * 100) + 50;
}

unsigned int getKnobY(byte row) {
  return row == 0 ? LAYOUT_KNOB_Y : 480 - LAYOUT_KNOB_Y;
}

void drawText(unsigned int x, unsigned int y, byte size, unsigned int color, const char* buffer) {
  tft.textMode();
  tft.textSetCursor(x, y);
  tft.textColor(color, RA8875_BLACK);
  tft.textEnlarge(size);
  tft.textWrite(buffer);
  tft.graphicsMode();
}

void drawKnob(byte row, byte channel, int currValue, int newValue) {
  int start, segments;
  unsigned int x = getKnobX(channel);
  unsigned int y = getKnobY(row);
  unsigned int color;

  // Draw value arc
  if (newValue != currValue) {
    if ((newValue >= 0 && newValue < currValue) || (newValue <= 0 && (newValue > currValue))) {
      // Remove segments
      start = newValue * 3;
      if (param[row] != PARAM_PAN) {
        start += 180;
      }
      segments = abs(currValue - newValue);
      color = RA8875_DARK_GREY;
    } else {
      // Add segments
      start = currValue * 3;
      if (param[row] != PARAM_PAN) {
        start += 180;
      }
      segments = abs(newValue - currValue);
      color = RA8875_WHITE;
    }
    fillArc2(x, y, start, segments, LAYOUT_KNOB_RADIUS, LAYOUT_KNOB_RADIUS, 10, color);
  }

  // Draw value text
  char buffer [4];
  sprintf(buffer, "%3i", newValue);
  drawText(x - 23, y - 20, RA8875_TEXT_MD, RA8875_WHITE, buffer);
}

void updateValue(byte row, byte channel, byte action) {
  int oldValue = state[param[row]][channel];
  int newValue;
  if (action == ACTION_INC) {
    newValue = oldValue + 1;
  }
  if (action == ACTION_DEC) {
    newValue = oldValue - 1;
  }
  state[param[row]][channel] = newValue;
  drawKnob(row, channel, oldValue, newValue);
}

void drawKnobBackground(byte row, byte channel) {
  // Draw thick grey circle
  unsigned int x = getKnobX(channel);
  unsigned int y = getKnobY(row);
  tft.fillCircle(x, y, LAYOUT_KNOB_RADIUS, RA8875_DARK_GREY);
  tft.fillCircle(x, y, LAYOUT_KNOB_RADIUS - 10, RA8875_BLACK);

  // Draw channel number
  char buffer [2];
  itoa(channel + 1, buffer, 10);
  y = row == 0 ? y + LAYOUT_KNOB_RADIUS + 3 : y - LAYOUT_KNOB_RADIUS - 35;
  drawText(x - 10, y, RA8875_TEXT_MD, RA8875_LIGHT_GREY, buffer);
}

void updateParam(byte row, byte action) {
  if (action == ACTION_INC && param[row] < PARAM_COUNT - 1) {
    param[row]++;
  }
  if (action == ACTION_DEC && param[row] > 0) {
    param[row]--;
  }

  // Draw all knobs & zero markers
  for (byte channel = 0; channel < CHANNEL_COUNT; channel++) {
    // Repaint knob bgs in case we're switching from unipolar to bipolar param
    drawKnobBackground(row, channel);
    drawKnob(row, channel, 0, state[param[row]][channel]);
  }

  // Draw param name
  drawText(8, row == 0 ? LAYOUT_PARAM_Y : 426 - LAYOUT_PARAM_Y, RA8875_TEXT_LG, RA8875_LIGHT_GREY, paramNames[param[0]]);
}

void handleKnob(unsigned int code) {
  byte action = code % 100;
  byte knob = code * 0.01; // 1-based including param knob

  if (knob == 1) {
    // Handle top param switch knob
    updateParam(0, action);
  } else if (knob == 10) {
    // Handle bottom param switch knob
    updateParam(1, action);
  } else {
    // Handle value knobs
    byte row = knob < 10 ? 0 : 1;
    byte channel = knob - (row == 0 ? 2 : 11);
    updateValue(row, channel, action);
  }
}

void drawBackground(void) {
  tft.drawFastHLine(0, LAYOUT_ROW_LINE_Y, 800, RA8875_LIGHT_GREY);
  tft.drawFastHLine(0, 480 - LAYOUT_ROW_LINE_Y, 800, RA8875_LIGHT_GREY);
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

  // Draw lines
  drawBackground();

  // Draw starting values
  updateParam(0, ACTION_NONE);
  updateParam(1, ACTION_NONE);

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
