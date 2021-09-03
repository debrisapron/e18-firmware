#include "Arduino.h"
#include "gfx.hpp"
#include "es9.hpp"

#define STATUS_INIT 0
#define STATUS_READY 1

#define ACTION_DEC 0
#define ACTION_INC 1

#define PARAM_VOL 0
#define PARAM_PAN 1
#define PARAM_EQ1_TYPE 2
#define PARAM_COUNT 6

#define CHANNEL_COUNT 8

const char *core_paramNames[] = {
  "VOL",
  "PAN",
  "EQ1 TYPE",
  "EQ1 FREQ",
  "EQ1 GAIN",
  "EQ1 Q",
};
const char *core_filterTypes[] = {
  "---",
  "LSH",
  "BND",
  "HSH",
  "LP1",
  "LP2",
  "HP1",
  "HP2"
};

byte core_status = STATUS_INIT;
byte core_param[] = {PARAM_VOL, PARAM_PAN};
byte core_state[][CHANNEL_COUNT] = {
  {0, 0, 0, 0, 0, 0, 0, 0},
  {128, 128, 128, 128, 128, 128, 128, 128},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0}
};

void core_getDisplayValue(char* buffer, byte param, byte value) {
  switch (param) {
    case PARAM_PAN: {
      int dispVal = value / 2 - 64;
      char sign = dispVal < 0 ? '-' : '+';
      sprintf(buffer, "%c%02d", sign, abs(dispVal));
      break;
    }
    case PARAM_EQ1_TYPE: {
      sprintf(buffer, "%s", core_filterTypes[value]);
      break;
    }
    default: {
      sprintf(buffer, "%03d", value / 2);
      break;
    }
  }
}

void core_drawDial(byte row, byte channel, byte oldValue, byte newValue) {
  char displayValueBuffer[4];
  byte param = core_param[row];
  bool isScalar = param != PARAM_EQ1_TYPE;
  core_getDisplayValue(displayValueBuffer, param, newValue);
  gfx_drawDial(row, channel, isScalar, oldValue, newValue, displayValueBuffer);
}

void core_setStereoVol(byte channel, byte vol, byte pan) {
  // ES9 gain level has a perceptual curve built in so pan law is weird
  float panRatio = pan / 255.0; // 0 - 1
  byte gainLeft = vol * (1 - pow(panRatio, 5)) * 0.5;
  byte gainRight = vol * (1 - pow(1 - panRatio, 5)) * 0.5;
  es9_setGain(channel, 0, gainLeft);
  es9_setGain(channel, 1, gainRight);
}

void core_updateHardware(byte param, byte channel, byte value) {
  switch (param) {
    case PARAM_VOL: {
      byte pan = core_state[PARAM_PAN][channel];
      core_setStereoVol(channel, value, pan);
      break;
    }
    case PARAM_PAN: {
      byte vol = core_state[PARAM_VOL][channel];
      core_setStereoVol(channel, vol, value);
      break;
    }
  }
}

void core_updateValue(byte row, byte channel, int direction) {
  byte param = core_param[row];
  byte oldValue = core_state[param][channel];
  bool isFilterType = param == PARAM_EQ1_TYPE;
  byte step = isFilterType ? 1 : 2;
  byte limit = isFilterType ? 7 : 254;
  int newValue;

  newValue = oldValue + step * direction;
  if (newValue < 0 || newValue > limit) return;
  core_state[param][channel] = newValue;

  core_drawDial(row, channel, oldValue, newValue);
  core_updateHardware(param, channel, newValue);
}

void core_drawRow(byte row, byte prevParam) {
  byte param = core_param[row];

  gfx_drawParamName(row, core_paramNames[param]);

  // Draw dials
  for (byte channel = 0; channel < CHANNEL_COUNT; channel++) {
    core_drawDial(row, channel, core_state[prevParam][channel], core_state[param][channel]);
  }
}

void core_updateParam(byte row, int direction) {
  byte oldParam = core_param[row];
  byte otherParam = core_param[1 - row];
  int newParam;

  // Increment or decrement row param, avoiding the other row's param
  newParam = oldParam + direction;
  if (newParam == otherParam) newParam += direction;
  if (newParam < 0 || newParam >= PARAM_COUNT) return;
  core_param[row] = newParam;

  core_drawRow(row, oldParam);
}

void core_handleEnc(byte enc, int action) {
  // For now handle only inc & dec
  if (action == 0) return;
  
  // -1 or 1
  int direction = action;

  if (enc == 0) {
    // Handle top param switch enc
    core_updateParam(0, direction);
  } else if (enc == 9) {
    // Handle bottom param switch enc
    core_updateParam(1, direction);
  } else {
    // Handle value encs
    byte row = enc < 9 ? 0 : 1;
    byte channel = enc - (row == 0 ? 1 : 10);
    core_updateValue(row, channel, direction);
  }
}

void core_setup(void) {
  gfx_setup();
  es9_setup();

  core_drawRow(0, core_param[0]);
  core_drawRow(1, core_param[1]);

  core_status = STATUS_READY;
}