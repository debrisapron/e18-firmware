#define STARTUP_WAIT 1000
#define ACTIVE_TIMEOUT_MS 1000

#define STATUS_INIT 0
#define STATUS_READY 1
#define PARAM_FILTER_TYPE_COUNT 8

byte core_status = STATUS_INIT;
byte core_paramIds[2];
E18State core_state;
unsigned long core_lastActiveMs = 0; // When set to zero, never go idle

void core_getDisplayValue(char* buffer, byte paramId, bool isDisabled, byte value) {
  if (isDisabled) {
    sprintf(buffer, "---");
    return;
  }

  byte kind = params[paramId].kind;
  switch (kind) {
    case PARAM_KIND_PAN: {
      int dispVal = value / 2 - 64;
      char sign = dispVal < 0 ? '-' : '+';
      sprintf(buffer, "%c%02d", sign, abs(dispVal));
      break;
    }
    case PARAM_KIND_FILTER_TYPE: {
      sprintf(buffer, "%s", filterTypes[value].name);
      break;
    }
    default: {
      sprintf(buffer, "%03d", value / 2);
      break;
    }
  }
}

bool core_getIsParamDisabled(byte paramId, byte channel) {
  byte parentParamId;
  switch(params[paramId].kind) {
    case PARAM_KIND_FILTER_FREQ: parentParamId = paramId - 1; break;
    case PARAM_KIND_FILTER_GAIN: parentParamId = paramId - 2; break;
    case PARAM_KIND_FILTER_Q: parentParamId = paramId - 3; break;
    default: parentParamId = 0; break;
  }
  return parentParamId && core_state[parentParamId][channel] == 0;
}

void core_drawDial(byte row, byte channel, byte oldValue, byte newValue) {
  char displayValueBuffer[4];
  byte paramId = core_paramIds[row];
  byte kind = params[paramId].kind;
  bool isScalar = kind != PARAM_KIND_FILTER_TYPE;
  bool isDisabled = core_getIsParamDisabled(paramId, channel);
  core_getDisplayValue(displayValueBuffer, kind, isDisabled, newValue);
  gfx_drawDial(row, channel, isScalar, isDisabled, oldValue, newValue, displayValueBuffer);
}

void core_updateValue(byte row, byte channel, int direction) {
  byte paramId = core_paramIds[row];
  if (core_getIsParamDisabled(paramId, channel)) return;
  
  byte oldValue = core_state[paramId][channel];
  bool isFilterType = params[paramId].kind == PARAM_KIND_FILTER_TYPE;
  byte step = isFilterType ? 1 : 2;
  byte limit = isFilterType ? (FILTER_TYPE_COUNT - 1) : 254;
  int newValue;

  newValue = oldValue + step * direction;
  if (newValue < 0 || newValue > limit) return;
  core_state[paramId][channel] = newValue;

  core_drawDial(row, channel, oldValue, newValue);
  es9_sendParam(paramId, channel, core_state);
  if (isFilterType && !oldValue != !newValue) {
    // If we're bypassing or activating an EQ, the other row might be showing
    // one of the EQs params and need to be disabled or enabled
    byte otherRow = 1 - row;
    byte otherParamId = core_paramIds[otherRow];
    byte otherRowValue = core_state[otherParamId][channel];
    core_drawDial(otherRow, channel, otherRowValue, otherRowValue);
  }
}

void core_drawRow(byte row, byte prevParamId) {
  byte paramId = core_paramIds[row];

  gfx_drawParamName(row, params[paramId].name);

  // Draw dials
  for (byte channel = 0; channel < CHANNEL_COUNT; channel++) {
    core_drawDial(row, channel, core_state[prevParamId][channel], core_state[paramId][channel]);
  }
}

void core_updateParam(byte row, int direction) {
  byte oldParamId = core_paramIds[row];
  byte otherParamId = core_paramIds[1 - row];
  int newParamId;

  // Increment or decrement row param, avoiding the other row's param
  newParamId = oldParamId + direction;
  if (newParamId == otherParamId) newParamId += direction;
  if (newParamId < 0 || newParamId >= PARAM_COUNT) return;
  core_paramIds[row] = newParamId;

  core_drawRow(row, oldParamId);
}

void core_handleEnc(byte enc, int action) {
  // For now handle only inc & dec
  if (action == 0) return;
  
  // -1 or 1
  int direction = action;

  if (enc == 0) {
    // Handle top param switch enc
    core_updateParam(ROW_TOP, direction);
  } else if (enc == 9) {
    // Handle bottom param switch enc
    core_updateParam(ROW_BOTTOM, direction);
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
  eep_load(core_state, core_paramIds);

  // Start encoders
  encs_setup();

  // Wait a second for ES9 to be ready
  delay(STARTUP_WAIT);

  // Start MIDI & sync state with ES9
  es9_setup(core_state);

  // Clear splash & draw static UI elements
  gfx_start();

  // Draw top & bottom dial rows
  core_drawRow(ROW_TOP, core_paramIds[ROW_TOP]);
  core_drawRow(ROW_BOTTOM, core_paramIds[ROW_BOTTOM]);

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
    eep_save(core_state, core_paramIds);
  }
}