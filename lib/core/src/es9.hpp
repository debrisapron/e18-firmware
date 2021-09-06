#include <MIDI.h>

#define SYSEX_4BYTE_HEADER 0x00, 0x21, 0x27, 0x19
#define SYSEX_SET_VIRTUAL_MIX 0x34
#define SYSEX_SET_FILTER 0x39

#define FREQ_MAX 0x00017F7Ful

#define MASTER_CHANNEL 0
#define AUX1_CHANNEL 2

MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, es9_midi1);

void es9_setup(void) {
  es9_midi1.begin(MIDI_CHANNEL_OMNI);
}

// from: 0-7 = inputs 1-8, 8-13 = aux return 9-14, 14-15 = bus 1-2
// to: 
//    (from 0-7)    0-1 = bus 1-2, 2-7 = aux send 1-6
//    (from 8-15)   8-9 = bus 1-2, 10-11 = main out 1-2, 12-13 = phones 1-2
void es9_setGain(byte from, byte to, byte gain) {
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

void es9_setFilter(byte channel, byte index, byte typeId, byte freq, byte q, byte gain) {
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

void es9_setStereoVol(byte channel, byte vol, byte pan, byte dest) {
  // ES9 gain level has a perceptual curve built in so pan law is weird
  float panRatio = pan / 255.0; // 0 - 1
  byte gainLeft = vol * (1 - pow(panRatio, 5)) * 0.5;
  byte gainRight = vol * (1 - pow(1 - panRatio, 5)) * 0.5;
  es9_setGain(channel, dest, gainLeft);
  es9_setGain(channel, dest + 1, gainRight);
}

void es9_setParam(byte paramId, byte channel, E18State state) {
  switch (paramId) {
    case PARAM_VOL:
    case PARAM_PAN:
    case PARAM_AUX1: 
    case PARAM_AUX1_PAN: {
      // Route channel to master
      byte vol = state[PARAM_VOL][channel];
      byte pan = state[PARAM_PAN][channel];
      es9_setStereoVol(channel, vol, pan, MASTER_CHANNEL);

      // Route channel to aux1
      byte aux1Vol = state[PARAM_AUX1][channel];
      byte aux1Pan = state[PARAM_AUX1_PAN][channel];
      // Simulate post-fader routing by multiplying by vol
      aux1Vol = aux1Vol * (vol / 255.0);
      es9_setStereoVol(channel, aux1Vol, aux1Pan, AUX1_CHANNEL);
      break;
    }
    case PARAM_EQ1_TYPE:
    case PARAM_EQ1_FREQ:
    case PARAM_EQ1_GAIN:
    case PARAM_EQ1_Q: {
      byte typeId = state[PARAM_EQ1_TYPE][channel];
      byte freq = state[PARAM_EQ1_FREQ][channel];
      byte gain = state[PARAM_EQ1_GAIN][channel];
      byte q = state[PARAM_EQ1_Q][channel];
      es9_setFilter(channel, 0, typeId, freq, q, gain);
    }
  }
}

void es9_setAllParams(E18State state) {
  for (byte channel = 0; channel < CHANNEL_COUNT; channel++) {
    es9_setParam(PARAM_VOL, channel, state);
  }
}