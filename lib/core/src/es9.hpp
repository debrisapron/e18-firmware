#include <MIDI.h>

#define SYSEX_4BYTE_HEADER 0x00, 0x21, 0x27, 0x19
#define SYSEX_SET_VIRTUAL_MIX 0x34
#define SYSEX_SET_FILTER 0x39

// Move this to core
// const byte es9_filterTypes[] = {
//   0x0, // LP1 disabled
//   0x9, // LSH
//   0xD, // BND
//   0xB, // HSH
//   0x1, // LP1
//   0x5, // LP2
//   0x3, // HP1
//   0x7  // HP2
// };

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

// void es9_setFilter(byte input, byte index, byte type, byte freq, byte q, byte gain) {
//   byte data[] = {
//     SYSEX_HEADER,
//     SYSEX_SET_FILTER,
//     input,
//     index,
//     es9_filterTypes[type],
//     freq, 0x00, 0x00,
//     q, 0x00, 0x00,
//     gain, 0x00, 0x00
//   };
//   es9_midi1.sendSysEx(17, data);
//   //F0 00 21 27 19 39 00 00 01 00 00 00 00 00 00 00 00 00 F7
// }

void es9_setStereoVol(byte channel, byte vol, byte pan) {
  // ES9 gain level has a perceptual curve built in so pan law is weird
  float panRatio = pan / 255.0; // 0 - 1
  byte gainLeft = vol * (1 - pow(panRatio, 5)) * 0.5;
  byte gainRight = vol * (1 - pow(1 - panRatio, 5)) * 0.5;
  es9_setGain(channel, 0, gainLeft);
  es9_setGain(channel, 1, gainRight);
}

void es9_setParam(byte param, byte channel, byte value, e18State state) {
  switch (param) {
    case PARAM_VOL: {
      byte pan = state[PARAM_PAN][channel];
      es9_setStereoVol(channel, value, pan);
      break;
    }
    case PARAM_PAN: {
      byte vol = state[PARAM_VOL][channel];
      es9_setStereoVol(channel, vol, value);
      break;
    }
  }
}

void es9_setAllParams(e18State state) {
  for (byte channel = 0; channel < CHANNEL_COUNT; channel++) {
    es9_setParam(PARAM_VOL, channel, state[PARAM_VOL][channel], state);
  }
}