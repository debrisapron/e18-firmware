#define MIDI_CREATE_INSTANCE(x, y, z)
#define MIDI_CHANNEL_OMNI 0

char __es9_sysexMessages[100][30];
byte __es9_idx; // Needs to be initialized to zero on every test

typedef struct {
  void begin(byte channel) {
  }
  void sendSysEx(unsigned inLength, const byte* inArray) {
    bool isValid = inArray[0] == 0x00 && inArray[1] == 0x21 && inArray[2] == 0x27 && inArray[3] == 0x19;
    TEST_ASSERT_TRUE_MESSAGE(isValid, "Sysex header should be 00 21 27 19");
    int strIdx = 0;
    for (byte i = 4; i < inLength; i++) {
      strIdx += sprintf(__es9_sysexMessages[__es9_idx] + strIdx, "%02X ", inArray[i]);
    }
    __es9_idx++;
  }
} MockMidi;

MockMidi es9_midi1;

#include <es9.hpp>