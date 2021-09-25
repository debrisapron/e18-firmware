#define STARTUP_WAIT 1000
#define ACTIVE_TIMEOUT_MS 1000
#define FLASH_DURATION 1000
#define MENU_ITEM_COUNT 3

typedef enum {
  MODE_MIX,
  MODE_MENU,
  MODE_EDIT_NAMES
} Mode;
typedef enum {
  PARAM_KIND_VOL,
  PARAM_KIND_PAN,
  PARAM_KIND_FILTER_TYPE,
  PARAM_KIND_FILTER_FREQ,
  PARAM_KIND_FILTER_GAIN,
  PARAM_KIND_FILTER_Q
} ParamKind;

typedef struct {
  const char* name;
  const ParamKind kind;
} Param;

const Param core_params[PARAM_COUNT] = {
  { "VOL" },
  { "PAN", PARAM_KIND_PAN },
  { "EQ1 TYPE", PARAM_KIND_FILTER_TYPE },
  { "EQ1 FREQ", PARAM_KIND_FILTER_FREQ },
  { "EQ1 GAIN", PARAM_KIND_FILTER_GAIN },
  { "EQ1 Q", PARAM_KIND_FILTER_Q },
  { "EQ2 TYPE", PARAM_KIND_FILTER_TYPE },
  { "EQ2 FREQ", PARAM_KIND_FILTER_FREQ },
  { "EQ2 GAIN", PARAM_KIND_FILTER_GAIN },
  { "EQ2 Q", PARAM_KIND_FILTER_Q },
  { "EQ3 TYPE", PARAM_KIND_FILTER_TYPE },
  { "EQ3 FREQ", PARAM_KIND_FILTER_FREQ },
  { "EQ3 GAIN", PARAM_KIND_FILTER_GAIN },
  { "EQ3 Q", PARAM_KIND_FILTER_Q },
  { "EQ4 TYPE", PARAM_KIND_FILTER_TYPE },
  { "EQ4 FREQ", PARAM_KIND_FILTER_FREQ },
  { "EQ4 GAIN", PARAM_KIND_FILTER_GAIN },
  { "EQ4 Q", PARAM_KIND_FILTER_Q },
  { "AUX1" },
  { "AUX1 PAN", PARAM_KIND_PAN },
  { "AUX2" },
  { "AUX2 PAN", PARAM_KIND_PAN },
  { "AUX3" },
  { "AUX3 PAN", PARAM_KIND_PAN },
  { "STATE" }
};
const char *core_menuItems[MENU_ITEM_COUNT][3] = {
  {"EDIT", "CHA", "NAMES"},
  {"", "SYNC", "ES-9"},
  {"", "FULL", "RESET"}
};
const char *core_okMenuItem[] = {" OK", "", ""};

Scene core_scene;
Scene core_sceneSlots[8];
bool core_fnDown[2] = {false, false};
bool core_fnWasUsedAsShift[2] = {false, false};
Mode core_mode = MODE_MIX;
char core_chanNamesBuffer[8][6];
byte core_chanNameInEdit;
unsigned long core_lastActiveMs = 0; // When set to zero, never go idle
unsigned long core_prevLastActiveMs = 0;
unsigned long core_lastShowedFlash = 0;

void core_setDirty(void) {
  core_prevLastActiveMs = core_lastActiveMs;
  core_lastActiveMs = millis();
}

// Hacked from the ES9 web config tool. Absolutely minging
void core_sevenBitToDb(char *buff, byte v) {
  if (v == 0) {
    sprintf(buff, "-inf");
    return;
  }
  if (v == 102) {
    sprintf(buff, "-0.5");
    return;
  }

	// Get dbs as a double
	double db = v >= 55
    ? -24.0 + 0.5 * (v - 55.0)
    : -72.0 + ( v - 1 ) * ( 72 - 24 ) / ( 55.0 - 1 );
  
	// Convert double to a string with max 2 signficant digits
  bool isFractional = db < 10 && db > -10 && !(v % 2);
  char sign = db < 0 ? '-' : '+';
	sprintf(buff, isFractional ? "%c%i.5" : "%c%-3i", sign, abs((int)db));
}

// Also hacked from the ES9 web config tool, but much nicer
void core_sevenBitToFreq(char *buff, byte v) {
  double min = 10.0;
  double mult = log(22000.0 / min) / 128.0;
  double freq = min * exp(mult * v);
  if (freq < 10000) {
    sprintf(buff, "%4i", (int)freq);
  } else {
    sprintf(buff, "%3ik", (int)(freq / 1000));
  }
}

void core_getDisplayValue(char* buff, byte paramId, bool isDisabled, byte value) {
  if (isDisabled) {
    sprintf(buff, "--- ");
    return;
  }

  byte kind = core_params[paramId].kind;
  switch (kind) {
    case PARAM_KIND_PAN: {
      int dispVal = value / 2 - 64;
      // Seems like you can't use '0' & '+' format flags together
      char sign = dispVal < 0 ? '-' : '+';
      sprintf(buff, "%c%02d ", sign, abs(dispVal));
      break;
    }
    case PARAM_KIND_FILTER_TYPE: {
      sprintf(buff, "%s ", filterTypes[value].name);
      break;
    }
    case PARAM_KIND_FILTER_FREQ: {
      core_sevenBitToFreq(buff, value / 2);
      break;
    }
    default: {
      core_sevenBitToDb(buff, value / 2);
      break;
    }
  }
}

bool core_getIsParamDisabled(byte paramId, byte channel) {
  byte parentParamId;
  switch(core_params[paramId].kind) {
    case PARAM_KIND_FILTER_FREQ: parentParamId = paramId - 1; break;
    case PARAM_KIND_FILTER_GAIN: parentParamId = paramId - 2; break;
    case PARAM_KIND_FILTER_Q: parentParamId = paramId - 3; break;
    default: parentParamId = 0; break;
  }
  return parentParamId && core_scene.mix[parentParamId][channel] == 0;
}

void core_drawDial(byte row, byte channel) {
  char displayValue[5];
  byte paramId = core_scene.pIds[row];
  byte value = core_scene.mix[paramId][channel];
  byte kind = core_params[paramId].kind;
  byte chaState = core_scene.mix[PARAM_CHA_STATE][channel];
  bool isScalar = kind != PARAM_KIND_FILTER_TYPE;
  bool isBipolar = kind == PARAM_KIND_PAN;
  bool isDisabled = core_getIsParamDisabled(paramId, channel);
  bool isSilent = false;
  if (chaState == CHA_STATE_MUTED) {
    isSilent = true;
  } else if (chaState == CHA_STATE_NORMAL) {
    for (byte cha = 0; cha < 8; cha++) {
      if (core_scene.mix[PARAM_CHA_STATE][cha] == CHA_STATE_SOLOED) {
        isSilent = true;
        break;
      }
    }
  }
  char *chanName = core_scene.chanNames[channel];
  core_getDisplayValue(displayValue, kind, isDisabled, value);
  gfx_drawDial(
    row,
    channel,
    value,
    chanName,
    displayValue,
    chaState,
    isScalar,
    isBipolar,
    isDisabled,
    isSilent
  );
}

void core_updateValue(byte row, byte channel, int direction, byte speed) {
  byte paramId = core_scene.pIds[row];
  if (core_getIsParamDisabled(paramId, channel)) return;
  
  byte oldValue = core_scene.mix[paramId][channel];
  byte kind = core_params[paramId].kind;
  byte step;
  byte limit;
  switch (kind) {
    case PARAM_KIND_FILTER_TYPE:
      step = 1;
      limit = FILTER_TYPE_COUNT - 1;
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

  // If we're bypassing/activating an EQ, the other row might
  // need to be redrawn
  if (kind == PARAM_KIND_FILTER_TYPE && !oldValue != !newValue) {
    byte otherRow = 1 - row;
    core_drawDial(otherRow, channel);
  }

  core_setDirty();
}

void core_drawRow(byte row) {
  byte paramId = core_scene.pIds[row];

  gfx_drawParamName(row, core_params[paramId].name);

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
  if (newParamId < 0 || newParamId >= PARAM_COUNT || newParamId == PARAM_CHA_STATE) return;
  core_scene.pIds[row] = newParamId;

  core_drawRow(row);
  core_setDirty();
}

void core_drawChanName(byte chan) {
  const char *chName[] = {"", "", core_chanNamesBuffer[chan]};
  gfx_drawMenuItem(
    0,
    chan,
    chName,
    chan == core_chanNameInEdit
  );
}

void core_updateChanName(byte charIndex, int delta) {
  int c = core_chanNamesBuffer[core_chanNameInEdit][charIndex] + delta;
  if (c < 32) c = 32;
  if (c > 126) c = 126;
  core_chanNamesBuffer[core_chanNameInEdit][charIndex] = (char)c;
  core_drawChanName(core_chanNameInEdit);
}

void core_drawChanNames(void) {
  for (byte chan = 0; chan < 8; chan++) {
    core_drawChanName(chan);
  }
}

void core_switchToMode(Mode mode) {
  gfx_clear();
  switch(mode) {
    case MODE_MIX:
      core_drawRow(0);
      core_drawRow(1);
      break;
    case MODE_MENU:
      for (byte i = 0; i < MENU_ITEM_COUNT; i++) {
        gfx_drawMenuItem(0, i, core_menuItems[i]);
      }
      break;
    case MODE_EDIT_NAMES:
      gfx_drawParamName(1, "EDIT NAMES");
      memcpy(&core_chanNamesBuffer, &(core_scene.chanNames), sizeof(core_chanNamesBuffer));
      core_chanNameInEdit = 0;
      core_drawChanNames();
      for (byte i = 0; i < 5; i++) {
        char label[6];
        sprintf(label, "CHR %i", i);
        const char *text[] = {label, "", ""};
        gfx_drawMenuItem(1, i, text, true);
      }
      gfx_drawMenuItem(1, 7, core_okMenuItem);
      break;
  }
  core_mode = mode;
}

void core_showFlash(const char* msg) {
  gfx_drawFlash(msg);
  core_lastShowedFlash = millis();
}

void core_saveChanNameChanges(void) {
  memcpy(&(core_scene.chanNames), &core_chanNamesBuffer, sizeof(core_chanNamesBuffer));
  core_switchToMode(MODE_MIX);
  core_showFlash("saved channel name changes");
  core_setDirty();
}

void core_saveScene(byte sceneId) {
  memcpy(&core_sceneSlots[sceneId], &core_scene, sizeof(Scene));
  eep_save(&core_sceneSlots[sceneId], sceneId + 1);

  char buffer[14];
  sprintf(buffer, "saved scene %i", sceneId + 1);
  core_showFlash(buffer);
}

void core_loadScene(byte sceneId) {
  memcpy(&core_scene, &core_sceneSlots[sceneId], sizeof(Scene));

  gfx_clear();
  core_drawRow(0);
  core_drawRow(1);

  es9_setup(core_scene.mix);

  char buffer[15];
  sprintf(buffer, "loaded scene %i", sceneId + 1);
  core_showFlash(buffer);
  core_setDirty();
}

void core_resetAllChaStates(void) {
  for (byte cha = 0; cha < 8; cha++) {
    core_scene.mix[PARAM_CHA_STATE][cha] = CHA_STATE_NORMAL;
  }
  core_drawRow(0);
  core_drawRow(1);
  for (byte cha = 0; cha < 8; cha++) {
    es9_sendParam(PARAM_CHA_STATE, cha, core_scene.mix);
  }
  core_setDirty();
}

void core_toggleChaState(byte channel, bool shift) {
  byte currState = core_scene.mix[PARAM_CHA_STATE][channel];
  byte newState = CHA_STATE_NORMAL;
  if (currState == CHA_STATE_NORMAL) {
    newState = shift ? CHA_STATE_SOLOED : CHA_STATE_MUTED;
  } else if (shift && currState == CHA_STATE_SOLOED) {
    for (byte cha = 0; cha < 8; cha++) {
      if (core_scene.mix[PARAM_CHA_STATE][cha] == CHA_STATE_SOLOED) {
        core_scene.mix[PARAM_CHA_STATE][cha] = CHA_STATE_NORMAL;
      }
    }
  }
  core_scene.mix[PARAM_CHA_STATE][channel] = newState;
  
  if (currState == CHA_STATE_SOLOED || newState == CHA_STATE_SOLOED) {
    core_drawRow(0);
    core_drawRow(1);
    for (byte cha = 0; cha < 8; cha++) {
      es9_sendParam(PARAM_CHA_STATE, cha, core_scene.mix);
    }
  } else {
    core_drawDial(0, channel);
    core_drawDial(1, channel);
    es9_sendParam(PARAM_CHA_STATE, channel, core_scene.mix);
  }

  core_setDirty();
}

void core_handleEncSwitch(byte enc, bool isPress) {
  bool isRelease = !isPress;

  // Handle top function switch
  if (enc == 0) {
    core_fnDown[0] = isPress;
    if (isRelease) {
      if (core_mode != MODE_MIX) {
        core_switchToMode(MODE_MIX);
      } else if(!core_fnWasUsedAsShift[0]) {
        core_switchToMode(MODE_MENU);
      }
      core_fnWasUsedAsShift[0] = false;
    }

  // Handle bottom function switch
  } else if (enc == 9 && core_mode == MODE_MIX) {
    core_fnDown[1] = isPress;
    if (isPress && core_fnDown[0]) {
      core_fnWasUsedAsShift[0] = true;
      core_fnWasUsedAsShift[1] = true;
      core_resetAllChaStates();
    } else if (isRelease) {
      core_fnWasUsedAsShift[1] = false;
    }

  // Handle bottom channel switches
  } else if (enc >= 10 && isPress) {
    if (core_mode == MODE_EDIT_NAMES && enc == 17) {
      core_saveChanNameChanges();
    } else if (core_mode == MODE_MIX) {
      core_toggleChaState(enc - 10, core_fnDown[1]);
    }

  // Handle top channel switches
  } else if (enc >= 1 && enc <= 8 && isRelease) {
    if (core_mode == MODE_MENU) {
      switch(enc) {
        case 1: core_switchToMode(MODE_EDIT_NAMES); break;
      }
    } else if (core_mode == MODE_EDIT_NAMES) {
      core_chanNameInEdit = enc - 1;
      core_drawChanNames();
    } else if (core_fnDown[0]) {
      core_fnWasUsedAsShift[0] = true;
      core_saveScene(enc - 1);
    } else {
      core_loadScene(enc - 1);
    }
  }
}

void core_handleEncRotate(byte enc, int direction, byte speed) {
  if (core_mode == MODE_EDIT_NAMES) {
    if (enc >= 10 && enc <= 14) {
      core_updateChanName(enc - 10, direction * speed);
    }
  }

  if (core_mode != MODE_MIX) return;

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

void core_seed(void) {
  for (byte i = 0; i < 9; i++) {
    Scene scene = {
      .pIds = {PARAM_PAN, PARAM_VOL},
      .chanNames = {"  1  ", "  2  ", "  3  ", "  4  ", "  5  ", "  6  ", "  7  ", "  8  "}
    };
    for (byte pId = 0; pId < PARAM_COUNT; pId++) {
      for (byte chan = 0; chan < CHANNEL_COUNT; chan++) {
        scene.mix[pId][chan] = (core_params[pId].kind == PARAM_KIND_PAN) ? 128 : 0;
      }
    }
    if (i == 0) {
      core_scene = scene;
    } else {
      core_sceneSlots[i - 1] = scene;
    }
    eep_save(&scene, i);
  }
}

void core_setup(void) {
  // Start graphics & show splash
  gfx_setup();

  // Get mix from EEPROM or intitialize
  bool isRestored = eep_load(&core_scene, 0);
  if (isRestored) {
    for (byte slot = 1; slot < 9; slot++) {
      eep_load(&core_sceneSlots[slot - 1], slot);
    }
  } else {
    core_seed();
  }

  // Start encoders
  encs_setup();

  // Wait a second for ES9 to be ready
  delay(STARTUP_WAIT);

  // Start MIDI & sync mix with ES9
  es9_setup(core_scene.mix);

  // Clear splash & draw static UI elements
  gfx_start();

  // Draw top & bottom dial rows
  core_drawRow(0);
  core_drawRow(1);

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
    eep_save(&core_scene, 0);
    core_showFlash("*");
  }
}