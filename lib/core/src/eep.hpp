#include <Arduino.h>
#include <EEPROM.h>
#include "shared.h"

#define E18_UID 0xB0, 0xF5, 0x66, 0x82
#define HEADER_LEN 5

const byte eep_header[HEADER_LEN] = {E18_UID, E18_VERSION};

void eep_load(E18State state) {
  // Read header
  bool headerOk = true;
  for (byte h = 0; h < HEADER_LEN; h++) {
    headerOk = headerOk && (EEPROM.read(h) == eep_header[h]);
  }

  if (!headerOk) {
    // Initialize state
    for (byte paramId = 0; paramId < PARAM_COUNT; paramId++) {
      for (byte channel = 0; channel < CHANNEL_COUNT; channel++) {
        state[paramId][channel] = (paramId == PARAM_PAN || paramId == PARAM_AUX1_PAN) ? 128 : 0;
      }
    }
    return;
  }

  // Read state
  for (byte paramId = 0; paramId < PARAM_COUNT; paramId++) {
    for (byte channel = 0; channel < CHANNEL_COUNT; channel++) {
      state[paramId][channel] = EEPROM.read(paramId * CHANNEL_COUNT + channel + HEADER_LEN);
    }
  }
}

void eep_save(E18State state) {
  // Write header
  for (byte h = 0; h < HEADER_LEN; h++) {
    EEPROM.update(h, eep_header[h]);
  }

  // Write state
  for (byte paramId = 0; paramId < PARAM_COUNT; paramId++) {
    for (byte channel = 0; channel < CHANNEL_COUNT; channel++) {
      EEPROM.update(paramId * CHANNEL_COUNT + channel + HEADER_LEN, state[paramId][channel]);
    }
  }
}