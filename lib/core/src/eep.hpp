#include <Arduino.h>
#include <EEPROM.h>

#define EEP_VERSION 5
#define EEP_UID 0xB0, 0xF5, 0x66, 0x82
#define HEADER_LEN 5

const byte eep_header[HEADER_LEN] = {EEP_UID, EEP_VERSION};

void eep_load(E18State state, byte rowParams[2]) {
  // Read header
  bool headerOk = true;
  for (byte h = 0; h < HEADER_LEN; h++) {
    headerOk = headerOk && (EEPROM.read(h) == eep_header[h]);
  }

  if (!headerOk) {
    // Initialize state
    rowParams[0] = PARAM_PAN;
    rowParams[1] = PARAM_VOL;
    for (byte paramId = 0; paramId < PARAM_COUNT; paramId++) {
      for (byte channel = 0; channel < CHANNEL_COUNT; channel++) {
        state[paramId][channel] = (paramId == PARAM_PAN || paramId == PARAM_AUX1_PAN) ? 128 : 0;
      }
    }
    return;
  }

  // Read state
  rowParams[0] = EEPROM.read(HEADER_LEN);
  rowParams[1] = EEPROM.read(HEADER_LEN + 1);
  for (byte paramId = 0; paramId < PARAM_COUNT; paramId++) {
    for (byte channel = 0; channel < CHANNEL_COUNT; channel++) {
      state[paramId][channel] = EEPROM.read(paramId * CHANNEL_COUNT + channel + HEADER_LEN + 2);
    }
  }
}

void eep_save(E18State state, byte rowParams[2]) {
  // Write header
  for (byte h = 0; h < HEADER_LEN; h++) {
    EEPROM.update(h, eep_header[h]);
  }

  // Write state
  EEPROM.update(HEADER_LEN, rowParams[0]);
  EEPROM.update(HEADER_LEN + 1, rowParams[1]);
  for (byte paramId = 0; paramId < PARAM_COUNT; paramId++) {
    for (byte channel = 0; channel < CHANNEL_COUNT; channel++) {
      EEPROM.update(paramId * CHANNEL_COUNT + channel + HEADER_LEN + 2, state[paramId][channel]);
    }
  }
}