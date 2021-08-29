#include "Arduino.h"
#include "Adafruit_GFX.h"
#include "Adafruit_RA8875.h"
#include "knobs.hpp"

// #include <EEPROM.h>
// #include "EEPROMAnything.h"

// Library only supports hardware SPI at this time
// Connect SCLK to Mega Digital #52
// Connect MISO to Mega Digital #50
// Connect MOSI to Mega Digital #51
// #define RA8875_INT 3
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

#define ACTION_NONE 255
#define ACTION_INC 1
#define ACTION_DEC 0

#define PARAM_VOL 0
#define PARAM_PAN 1
#define PARAM_COUNT 2
#define CHANNEL_COUNT 8

Adafruit_RA8875 core_tft = Adafruit_RA8875(RA8875_CS, RA8875_RESET);
byte core_status = STATUS_INIT;
byte core_param[] = {PARAM_VOL, PARAM_PAN};
int core_state[][CHANNEL_COUNT] = {
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0}
};
const char *core_paramNames[] = {"VOL", "PAN"};

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

void core_fillArc2(unsigned int x, unsigned int y, int start_angle, int seg_count, int rx, int ry, int w, unsigned int colour)
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

    core_tft.fillTriangle(x0, y0, x1, y1, x2, y2, colour);
    core_tft.fillTriangle(x1, y1, x2, y2, x3, y3, colour);

    // Copy segment end to segment start for next segment
    x0 = x2;
    y0 = y2;
    x1 = x3;
    y1 = y3;
  }
}

unsigned int core_getKnobX(byte channel) {
  return (channel * 100) + 50;
}

unsigned int core_getKnobY(byte row) {
  return row == 0 ? LAYOUT_KNOB_Y : 480 - LAYOUT_KNOB_Y;
}

void core_drawText(unsigned int x, unsigned int y, byte size, unsigned int color, const char* buffer) {
  core_tft.textMode();
  core_tft.textSetCursor(x, y);
  core_tft.textColor(color, RA8875_BLACK);
  core_tft.textEnlarge(size);
  core_tft.textWrite(buffer);
  core_tft.graphicsMode();
}

void core_drawKnob(byte row, byte channel, int currValue, int newValue) {
  int start, segments;
  unsigned int x = core_getKnobX(channel);
  unsigned int y = core_getKnobY(row);
  unsigned int color;

  // Draw value arc
  if (newValue != currValue) {
    if ((newValue >= 0 && newValue < currValue) || (newValue <= 0 && (newValue > currValue))) {
      // Remove segments
      start = newValue * 3;
      if (core_param[row] != PARAM_PAN) {
        start += 180;
      }
      segments = abs(currValue - newValue);
      color = RA8875_DARK_GREY;
    } else {
      // Add segments
      start = currValue * 3;
      if (core_param[row] != PARAM_PAN) {
        start += 180;
      }
      segments = abs(newValue - currValue);
      color = RA8875_WHITE;
    }
    core_fillArc2(x, y, start, segments, LAYOUT_KNOB_RADIUS, LAYOUT_KNOB_RADIUS, 10, color);
  }

  // Draw value text
  char buffer [4];
  sprintf(buffer, "%3i", newValue);
  core_drawText(x - 23, y - 20, RA8875_TEXT_MD, RA8875_WHITE, buffer);
}

void core_updateValue(byte row, byte channel, byte action) {
  int oldValue = core_state[core_param[row]][channel];
  int newValue;
  if (action == ACTION_INC) {
    newValue = oldValue + 1;
  }
  if (action == ACTION_DEC) {
    newValue = oldValue - 1;
  }
  core_state[core_param[row]][channel] = newValue;
  core_drawKnob(row, channel, oldValue, newValue);
}

void core_drawKnobBackground(byte row, byte channel) {
  // Draw thick grey circle
  unsigned int x = core_getKnobX(channel);
  unsigned int y = core_getKnobY(row);
  core_tft.fillCircle(x, y, LAYOUT_KNOB_RADIUS, RA8875_DARK_GREY);
  core_tft.fillCircle(x, y, LAYOUT_KNOB_RADIUS - 10, RA8875_BLACK);

  // Draw channel number
  char buffer [2];
  itoa(channel + 1, buffer, 10);
  y = row == 0 ? y + LAYOUT_KNOB_RADIUS + 3 : y - LAYOUT_KNOB_RADIUS - 35;
  core_drawText(x - 10, y, RA8875_TEXT_MD, RA8875_LIGHT_GREY, buffer);
}

void core_updateParam(byte row, byte action) {
  if (action == ACTION_INC && core_param[row] < PARAM_COUNT - 1) {
    core_param[row]++;
  }
  if (action == ACTION_DEC && core_param[row] > 0) {
    core_param[row]--;
  }

  // Draw all knobs & zero markers
  for (byte channel = 0; channel < CHANNEL_COUNT; channel++) {
    // Repaint knob bgs in case we're switching from unipolar to bipolar param
    core_drawKnobBackground(row, channel);
    core_drawKnob(row, channel, 0, core_state[core_param[row]][channel]);
  }

  // Draw param name
  core_drawText(8, row == 0 ? LAYOUT_PARAM_Y : 426 - LAYOUT_PARAM_Y, RA8875_TEXT_LG, RA8875_LIGHT_GREY, core_paramNames[core_param[0]]);
}

void core_handleKnob(unsigned int code) {
  byte action = code % 10;
  byte knob = code * 0.1; // 1-based including param knob

  // For now handle only inc & dec
  if (action > 1) { return; }

  if (knob == 1) {
    // Handle top param switch knob
    core_updateParam(0, action);
  } else if (knob == 10) {
    // Handle bottom param switch knob
    core_updateParam(1, action);
  } else {
    // Handle value knobs
    byte row = knob < 10 ? 0 : 1;
    byte channel = knob - (row == 0 ? 2 : 11);
    core_updateValue(row, channel, action);
  }
}

void core_drawBackground(void) {
  core_tft.drawFastHLine(0, LAYOUT_ROW_LINE_Y, 800, RA8875_LIGHT_GREY);
  core_tft.drawFastHLine(0, 480 - LAYOUT_ROW_LINE_Y, 800, RA8875_LIGHT_GREY);
}

void core_setup(void) {
  // Start TFT
  bool ok = core_tft.begin(RA8875_800x480);
  if (!ok) {
    Serial.begin(9600);
    Serial.println("RA8875 Not Found!");
    return;
  }
  core_tft.displayOn(true);
  core_tft.GPIOX(true); // Enable TFT - display enable tied to GPIOX
  core_tft.PWM1config(true, RA8875_PWM_CLK_DIV1024); // PWM output for backlight
  core_tft.PWM1out(255);

  // Draw lines
  core_drawBackground();

  // Draw starting values
  core_updateParam(0, ACTION_NONE);
  core_updateParam(1, ACTION_NONE);

  knobs_setup();

  core_status = STATUS_READY;
}

void core_loop(void) {
  unsigned int code = knobs_read();
  if (code != 0) {
    core_handleKnob(code);
  }
}

  // for (int i = 50; i < 74; i++) 
  // {
  //   MIDI.sendNoteOn(i, 64, 1); 
  //   delay(75); 
  //   MIDI.sendNoteOff(i, 64, 1); 
  //   delay(75); 
  // }