#include <MIDI.h>

#define SYSEX_HEADER 0x00, 0x21, 0x27, 0x19
#define SYSEX_SET_VIRTUAL_MIX 0x34

MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, midi1);

void es9_setup(void) {
  midi1.begin(MIDI_CHANNEL_OMNI);
}

// from: 1-8 = inputs 1-8, 9-14 = aux return 9-14, 15-16 = bus 1-2
// to: 
//    (from 1-8)    1-2 = bus 1-2, 3-8 = aux send 1-6
//    (from 9-16)   9-10 = bus 1-2, 11-12 = main out 1-2, 13-14 = phones 1-2
void es9_setGain(byte from, byte to, byte gain) {
  // TODO Error handling
  // if (from < 9 && to > 8)
  // if (from > 8 && to < 9)
  byte mixNo = ((to - 1) * 8) + ((from - 1) % 8);
  byte data[] = {SYSEX_HEADER, SYSEX_SET_VIRTUAL_MIX, mixNo, gain};
  midi1.sendSysEx(7, data);
}