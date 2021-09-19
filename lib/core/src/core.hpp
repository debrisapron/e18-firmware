#define STARTUP_WAIT 1000
#define ACTIVE_TIMEOUT_MS 1000
#define FLASH_DURATION 1000

#define PARAM_FILTER_TYPE_COUNT 8

Scene core_scene;
Scene core_sceneSlots[8];
bool core_fnDown[2] = {0, 0};
unsigned long core_lastActiveMs = 0; // When set to zero, never go idle
unsigned long core_prevLastActiveMs = 0;
unsigned long core_lastShowedFlash = 0;

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
  return parentParamId && core_scene.mix[parentParamId][channel] == 0;
}

void core_drawDial(byte row, byte channel) {
  char displayValue[4];
  byte paramId = core_scene.pIds[row];
  byte value = core_scene.mix[paramId][channel];
  byte kind = params[paramId].kind;
  bool isScalar = kind != PARAM_KIND_FILTER_TYPE;
  bool isDisabled = core_getIsParamDisabled(paramId, channel);
  bool isMuted = core_scene.mix[PARAM_MUTE][channel];
  core_getDisplayValue(displayValue, kind, isDisabled, value);
  gfx_drawDial(row, channel, value, displayValue, isScalar, isDisabled, isMuted);
}

void core_updateValue(byte row, byte channel, int direction, byte speed) {
  byte paramId = core_scene.pIds[row];
  if (core_getIsParamDisabled(paramId, channel)) return;
  
  byte oldValue = core_scene.mix[paramId][channel];
  byte kind = params[paramId].kind;
  byte step;
  byte limit;
  switch (kind) {
    case PARAM_KIND_FILTER_TYPE:
      step = 1;
      limit = FILTER_TYPE_COUNT - 1;
      speed = 1;
      break;
    case PARAM_KIND_MUTE:
      step = 1;
      limit = 1;
      speed = 1;
      break;
    default:
      step = 2;
      limit = 254;
  }
  int newValue;

  // Starting with the passed-in speed, try decreasing speed values until we get
  // a delta that fits within the parameter range. If we get to zero, we have
  // hit the limit so return.
  // TODO Figure out a simpler way to do this, I am very intelligent
  for (byte attemptedSpeed = speed; attemptedSpeed >= 0; attemptedSpeed--) {
    if (attemptedSpeed == 0) return;
    newValue = oldValue + direction * step * attemptedSpeed;
    if (newValue >= 0 && newValue <= limit) break;
  }
  
  core_scene.mix[paramId][channel] = newValue;
  core_drawDial(row, channel);
  es9_sendParam(paramId, channel, core_scene.mix);

  // If we're toggling mute, or bypassing/activating an EQ, the other row might
  // need to be redrawn
  if (kind == PARAM_KIND_MUTE || (kind == PARAM_KIND_FILTER_TYPE && !oldValue != !newValue)) {
    byte otherRow = 1 - row;
    core_drawDial(otherRow, channel);
  }
}

void core_drawRow(byte row) {
  byte paramId = core_scene.pIds[row];

  gfx_drawParamName(row, params[paramId].name);

  // Draw dials
  for (byte channel = 0; channel < CHANNEL_COUNT; channel++) {
    core_drawDial(row, channel);
  }
}

void core_updateParam(byte row, int direction) {
  byte oldParamId = core_scene.pIds[row];
  byte otherParamId = core_scene.pIds[1 - row];
  int newParamId;

  // Increment or decrement row param, avoiding the other row's param
  newParamId = oldParamId + direction;
  if (newParamId == otherParamId) newParamId += direction;
  if (newParamId < 0 || newParamId >= PARAM_COUNT) return;
  core_scene.pIds[row] = newParamId;

  core_drawRow(row);
}

void core_showFlash(const char* msg) {
  gfx_drawFlash(msg);
  core_lastShowedFlash = millis();
}

void core_saveScene(byte sceneId) {
  memcpy(&core_sceneSlots[sceneId], &core_scene, sizeof(Scene));

  char buffer[14];
  sprintf(buffer, "saved scene %i", sceneId + 1);
  core_showFlash(buffer);
}

void core_loadScene(byte sceneId) {
  memcpy(&core_scene, &core_sceneSlots[sceneId], sizeof(Scene));

  core_drawRow(ROW_TOP);
  core_drawRow(ROW_BOTTOM);

  es9_setup(core_scene.mix);

  char buffer[15];
  sprintf(buffer, "loaded scene %i", sceneId + 1);
  core_showFlash(buffer);
}

void core_toggleMute(byte channel) {
  core_scene.mix[PARAM_MUTE][channel] = !core_scene.mix[PARAM_MUTE][channel];
  core_drawDial(ROW_TOP, channel);
  core_drawDial(ROW_BOTTOM, channel);
  es9_sendParam(PARAM_MUTE, channel, core_scene.mix);
}

void core_handleEncSwitch(byte enc, bool isPress) {
  if (enc == 0) {
    core_fnDown[0] = isPress;
  } else if (enc >= 10 && isPress) {
    core_toggleMute(enc - 10);
  } else if (enc >= 1 && enc <= 8 && !isPress) {
    if (core_fnDown[0]) {
      core_saveScene(enc - 1);
    } else {
      core_loadScene(enc - 1);
    }
  }
}

void core_handleEncRotate(byte enc, int direction, byte speed) {
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
    core_updateValue(row, channel, direction, speed);
  }
}

void core_seed() {
  for (byte i = 0; i < 9; i++) {
    Scene scene = {
      .pIds = {PARAM_PAN, PARAM_VOL},
    };
    for (byte pId = 0; pId < PARAM_COUNT; pId++) {
      for (byte ch = 0; ch < CHANNEL_COUNT; ch++) {
        scene.mix[pId][ch] = (params[pId].kind == PARAM_KIND_PAN) ? 128 : 0;
      }
    }
    if (i == 0) {
      core_scene = scene;
    } else {
      core_sceneSlots[i - 1] = scene;
    }
  }
}

void core_setup(void) {
  // Start graphics & show splash
  gfx_setup();

  // Get mix from EEPROM or intitialize
  bool isRestored = eep_load(&core_scene, core_sceneSlots);
  if (!isRestored) core_seed();

  // Start encoders
  encs_setup();

  // Wait a second for ES9 to be ready
  delay(STARTUP_WAIT);

  // Start MIDI & sync mix with ES9
  es9_setup(core_scene.mix);

  // Clear splash & draw static UI elements
  gfx_start();

  // Draw top & bottom dial rows
  core_drawRow(ROW_TOP);
  core_drawRow(ROW_BOTTOM);

  core_showFlash(isRestored ? "restored state from EEPROM" : "initialized state");
}

void core_loop(void) {
  unsigned int encCode = encs_read();
  unsigned long now = millis();

  if (core_lastShowedFlash != 0 && now - core_lastShowedFlash > FLASH_DURATION) {
    core_lastShowedFlash = 0;
    gfx_clearFlash();
  }
  
  if (encCode) {
    byte encIndex = encCode >> 8;
    byte encAction = encCode & 0xFF;
    if (encAction == ENC_ACTION_DEC || encAction == ENC_ACTION_INC) {
      byte speed = 1;
      if (core_prevLastActiveMs != 0) {
        unsigned long msSinceLastActive = now - core_lastActiveMs;
        unsigned long msSincePrevLastActive = now - core_prevLastActiveMs;
        if (msSinceLastActive < 100 && msSincePrevLastActive < 200) {
          speed = 4;
        } else if (msSinceLastActive < 200 && msSincePrevLastActive < 400) {
          speed = 2;
        }
      }
      core_prevLastActiveMs = core_lastActiveMs;
      core_lastActiveMs = now;
      int encDirection = encAction == ENC_ACTION_INC ? 1 : -1;
      core_handleEncRotate(encIndex, encDirection, speed);
    } else {
      bool isPress = encAction == ENC_ACTION_PRESS;
      core_handleEncSwitch(encIndex, isPress);
    }
    return;
  }

  if (core_lastActiveMs != 0 && (now - core_lastActiveMs) > ACTIVE_TIMEOUT_MS) {
    core_lastActiveMs = 0;
    core_prevLastActiveMs = 0;
    eep_save(&core_scene, core_sceneSlots);
    core_showFlash("saved state to EEPROM");
  }
}