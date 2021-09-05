#include <Arduino.h>
#include "shared.h"
#include "encs.hpp"
#include "gfx.hpp"
#include "es9.hpp"
#include "eep.hpp"

#define STATUS_INIT 0
#define STATUS_READY 1
#define ACTIVE_TIMEOUT_MS 1000
#define ULONG_MAX (0UL - 1UL)

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
byte core_params[] = {PARAM_VOL, PARAM_PAN};
e18State core_state;
unsigned long core_lastActiveMs = 0; // When set to zero, never go idle

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
  byte param = core_params[row];
  bool isScalar = param != PARAM_EQ1_TYPE;
  core_getDisplayValue(displayValueBuffer, param, newValue);
  gfx_drawDial(row, channel, isScalar, oldValue, newValue, displayValueBuffer);
}

void core_updateValue(byte row, byte channel, int direction) {
  byte param = core_params[row];
  byte oldValue = core_state[param][channel];
  bool isFilterType = param == PARAM_EQ1_TYPE;
  byte step = isFilterType ? 1 : 2;
  byte limit = isFilterType ? 7 : 254;
  int newValue;

  newValue = oldValue + step * direction;
  if (newValue < 0 || newValue > limit) return;
  core_state[param][channel] = newValue;

  core_drawDial(row, channel, oldValue, newValue);
  es9_setParam(param, channel, newValue, core_state);
}

void core_drawRow(byte row, byte prevParam) {
  byte param = core_params[row];

  gfx_drawParamName(row, core_paramNames[param]);

  // Draw dials
  for (byte channel = 0; channel < CHANNEL_COUNT; channel++) {
    core_drawDial(row, channel, core_state[prevParam][channel], core_state[param][channel]);
  }
}

void core_updateParam(byte row, int direction) {
  byte oldParam = core_params[row];
  byte otherParam = core_params[1 - row];
  int newParam;

  // Increment or decrement row param, avoiding the other row's param
  newParam = oldParam + direction;
  if (newParam == otherParam) newParam += direction;
  if (newParam < 0 || newParam >= PARAM_COUNT) return;
  core_params[row] = newParam;

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
  // Start graphics & show splash
  gfx_setup();

  // Get state from EEPROM
  eep_load(core_state);

  // Start encoders
  encs_setup();

  // Wait a second for ES9 to be ready
  delay(1000);

  // Start MIDI & sync state with ES9
  es9_setup();
  es9_setAllParams(core_state);

  // Clear splash & draw static UI elements
  gfx_start();

  // Draw top & bottom dial rows
  core_drawRow(0, core_params[0]);
  core_drawRow(1, core_params[1]);

  // LET'S ROCK
  core_status = STATUS_READY;
}

void core_loop(void) {
  encs_read();

  if (encs_newIndex > -1) {
    core_handleEnc(encs_newIndex, encs_newAction);
    core_lastActiveMs = millis();
    return;
  }

  if (core_lastActiveMs != 0 && millis() > core_lastActiveMs + ACTIVE_TIMEOUT_MS) {
    core_lastActiveMs = 0;
    eep_save(core_state);
  }
}