#include "Arduino.h"
#include "gfx.hpp"

// #include <EEPROM.h>
// #include "EEPROMAnything.h"

#define STATUS_INIT 0
#define STATUS_READY 1

#define ACTION_DEC 0
#define ACTION_INC 1

#define PARAM_VOL 0
#define PARAM_PAN 1
#define PARAM_COUNT 2
#define CHANNEL_COUNT 8

byte core_status = STATUS_INIT;
byte core_param[] = {PARAM_VOL, PARAM_PAN};
byte core_state[][CHANNEL_COUNT] = {
  {0, 0, 0, 0, 0, 0, 0, 0},
  {128, 128, 128, 128, 128, 128, 128, 128}
};
const char *core_paramNames[] = {"VOL", "PAN"};

bool core_getIsBipolar(byte row) {
  return core_param[row] == PARAM_PAN;
}

void core_updateValue(byte row, byte channel, byte action) {
  byte oldValue = core_state[core_param[row]][channel];
  byte newValue;
  if (action == ACTION_INC) {
    if (oldValue >= 254) return;
    newValue = oldValue + 2;
  }
  if (action == ACTION_DEC) {
    if (oldValue == 0) return;
    newValue = oldValue - 2;
  }
  core_state[core_param[row]][channel] = newValue;
  gfx_drawKnob(row, channel, core_getIsBipolar(row), oldValue, newValue);
}

void core_drawRow(byte row, const byte* oldValues, const byte* newValues) {
  const char *paramName = core_paramNames[core_param[row]];
  bool isBipolar = core_getIsBipolar(row);
  gfx_drawRow(row, paramName, isBipolar, oldValues, newValues);
}

void core_updateParam(byte row, byte action) {
  byte oldParam = core_param[row];
  byte newParam;
  if (action == ACTION_INC) {
    if (oldParam == PARAM_COUNT - 1) return;
    newParam = oldParam + 1;
  }
  if (action == ACTION_DEC) {
    if (oldParam == 0) return;
    newParam = oldParam - 1;
  }
  core_param[row] = newParam;
  core_drawRow(row, core_state[oldParam], core_state[newParam]);
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

void core_setup(void) {
  gfx_setup();

  core_drawRow(0, core_state[0], core_state[0]);
  core_drawRow(1, core_state[1], core_state[1]);

  core_status = STATUS_READY;
}

  // for (int i = 50; i < 74; i++) 
  // {
  //   MIDI.sendNoteOn(i, 64, 1); 
  //   delay(75); 
  //   MIDI.sendNoteOff(i, 64, 1); 
  //   delay(75); 
  // }