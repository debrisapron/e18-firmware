#define PARAM_EQ_START 2
#define PARAM_EQ_COUNT 4
#define PARAM_EQ_PARAM_COUNT 4
#define PARAM_EQ_END 17
#define PARAM_AUX_START 18
#define PARAM_AUX_COUNT 3
#define PARAM_AUX_PARAM_COUNT 2
#define PARAM_AUX_END 23

#define SYSEX_4BYTE_HEADER 0x00, 0x21, 0x27, 0x19
#define SYSEX_SET_VIRTUAL_MIX 0x34
#define SYSEX_SET_FILTER 0x39

#define FREQ_MAX 0x00017F7Ful

#define MASTER_CHANNEL 0
#define AUX1_CHANNEL 2

MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, es9_midi1);

// from: 0-7 = inputs 1-8, 8-13 = aux return 9-14, 14-15 = bus 1-2
// to: 
//    (from 0-7)    0-1 = bus 1-2, 2-7 = aux send 1-6
//    (from 8-15)   8-9 = bus 1-2, 10-11 = main out 1-2, 12-13 = phones 1-2
void es9_sendGain(byte from, byte to, byte gain) {
  // TODO Error handling
  // if (from < 9 && to > 8)
  // if (from > 8 && to < 9)
  byte mixNo = to * 8 + (from % 8);
  byte data[] = {SYSEX_4BYTE_HEADER, SYSEX_SET_VIRTUAL_MIX, mixNo, gain};
  es9_midi1.sendSysEx(7, data);
}

// Takes a byte from 0-255 & returns the three-byte array the ES9 uses for hi-res values
void es9_getThreeByteValue(byte out[3], byte v) {
  unsigned int _v = (v / 255.0) * 32767;
  out[0] = (_v >> 14) & 0x03;
  out[1] = (_v >> 7) & 0x7F;
  out[2] = _v & 0x7F;
}

void es9_sendFilter(byte channel, byte index, byte typeId, byte freq, byte q, byte gain) {
  byte freqBytes[3];
  es9_getThreeByteValue(freqBytes, freq);
  byte qBytes[3];
  es9_getThreeByteValue(qBytes, q);
  byte gainBytes[3];
  es9_getThreeByteValue(gainBytes, gain);
  byte data[] = {
    SYSEX_4BYTE_HEADER,
    SYSEX_SET_FILTER,
    channel,
    index,
    filterTypes[typeId].es9Id,
    freqBytes[0], freqBytes[1], freqBytes[2],
    qBytes[0], qBytes[1], qBytes[2],
    gainBytes[0], gainBytes[1], gainBytes[2],
  };
  es9_midi1.sendSysEx(17, data);
}

void es9_sendStereoVol(byte channel, byte vol, byte pan, byte dest) {
  // ES9 gain level has a perceptual curve built in so pan law is weird
  float panRatio = pan / 255.0; // 0 - 1
  byte gainLeft = vol * (1 - pow(panRatio, 5)) * 0.5;
  byte gainRight = vol * (1 - pow(1 - panRatio, 5)) * 0.5;
  es9_sendGain(channel, dest, gainLeft);
  es9_sendGain(channel, dest + 1, gainRight);
}

void es9_sendAuxStereoVol(byte channel, E18State state, byte auxNo) {
  bool isMuted = state[PARAM_MUTE][channel];
  byte auxVolParamId = PARAM_AUX_START + auxNo * PARAM_AUX_PARAM_COUNT;
  byte auxVol = state[auxVolParamId][channel];
  byte auxPan = state[auxVolParamId + 1][channel];

  byte channelVol = isMuted ? 0 : state[PARAM_VOL][channel];
  auxVol = auxVol * (channelVol / 255.0);

  byte auxOut = AUX1_CHANNEL + auxNo * PARAM_AUX_PARAM_COUNT;
  es9_sendStereoVol(channel, auxVol, auxPan, auxOut);
}

void es9_sendEq(byte channel, E18State state, byte eqNo) {
  byte eqTypeParamId = PARAM_EQ_START + eqNo * PARAM_EQ_PARAM_COUNT;
  byte typeId = state[eqTypeParamId][channel];
  byte freq = state[eqTypeParamId + 1][channel];
  byte gain = state[eqTypeParamId + 2][channel];
  byte q = state[eqTypeParamId + 3][channel];
  es9_sendFilter(channel, eqNo, typeId, freq, q, gain);
}

void es9_sendChannelStereoVols(byte channel, E18State state) {
  bool isMuted = state[PARAM_MUTE][channel];
  byte vol = isMuted ? 0 : state[PARAM_VOL][channel];
  byte pan = state[PARAM_PAN][channel];
  es9_sendStereoVol(channel, vol, pan, MASTER_CHANNEL);
  for (byte auxNo = 0; auxNo < PARAM_AUX_COUNT; auxNo++) {
    es9_sendAuxStereoVol(channel, state, auxNo);
  }
}

void es9_sendParam(byte paramId, byte channel, E18State state) {
  // Handle all mix routings
  if (paramId == PARAM_VOL || paramId == PARAM_PAN || paramId == PARAM_MUTE) {
    es9_sendChannelStereoVols(channel, state);
  } else if (paramId >= PARAM_EQ_START && paramId <= PARAM_EQ_END) {
    byte eqNo = (paramId - PARAM_EQ_START) / PARAM_EQ_PARAM_COUNT;
    es9_sendEq(channel, state, eqNo);
  } else {
    byte auxNo = (paramId - PARAM_AUX_START) / PARAM_AUX_PARAM_COUNT;
    es9_sendAuxStereoVol(channel, state, auxNo);
  }
}

void es9_sendAllParams(E18State state) {
  for (byte channel = 0; channel < CHANNEL_COUNT; channel++) {
    es9_sendChannelStereoVols(channel, state);
    for (byte eqNo = 0; eqNo < PARAM_EQ_COUNT; eqNo++) {
      es9_sendEq(channel, state, eqNo);
    }
    // No need to send auxes as they are handled by es9_sendChannelStereoVols
  }
}

void es9_setup(E18State state) {
  es9_midi1.begin(MIDI_CHANNEL_OMNI);
  es9_sendAllParams(state);
}