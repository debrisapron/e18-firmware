#include "Arduino.h"
#include "knobs.hpp"
#include "gfx.hpp"

// #include <EEPROM.h>
// #include "EEPROMAnything.h"

#define STATUS_INIT 0
#define STATUS_READY 1

#define ACTION_NONE 255
#define ACTION_INC 1
#define ACTION_DEC 0

#define PARAM_VOL 0
#define PARAM_PAN 1
#define PARAM_COUNT 2
#define CHANNEL_COUNT 8

byte core_status = STATUS_INIT;
byte core_param[] = {PARAM_VOL, PARAM_PAN};
int core_state[][CHANNEL_COUNT] = {
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0}
};
const char *core_paramNames[] = {"VOL", "PAN"};

bool core_getIsBipolar(byte row) {
  return core_param[row] == PARAM_PAN;
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
  gfx_drawKnob(row, channel, core_getIsBipolar(row), oldValue, newValue);
}

void core_updateParam(byte row, byte action) {
  if (action == ACTION_INC && core_param[row] < PARAM_COUNT - 1) {
    core_param[row]++;
  }
  if (action == ACTION_DEC && core_param[row] > 0) {
    core_param[row]--;
  }

  // Draw all knobs & zero markers
  bool isBipolar = core_getIsBipolar(row);
  for (byte channel = 0; channel < CHANNEL_COUNT; channel++) {
    // Repaint knob bgs in case we're switching from unipolar to bipolar param
    gfx_drawKnobBackground(row, channel);
    gfx_drawKnob(row, channel, isBipolar, 0, core_state[core_param[row]][channel]);
  }

  gfx_drawParamName(row, core_paramNames[core_param[0]]);
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
  knobs_setup();
  gfx_setup();

  // Initialize params & param values
  core_updateParam(0, ACTION_NONE);
  core_updateParam(1, ACTION_NONE);

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