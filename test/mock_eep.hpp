byte* __eep_data;

typedef struct {
  uint8_t read(int idx) {
    return __eep_data[idx];
  }
  void update(int idx, uint8_t val) {
    //
  }
} MockEEPROM;

MockEEPROM EEPROM;

#include <eep.hpp>

byte __EEP_EMPTY[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
byte __EEP_DATA[CHANNEL_COUNT * PARAM_COUNT + 7] = {
  0xB0, 0xF5, 0x66, 0x82, EEP_VERSION, // Header 
  3, 2                                 // Selected params
};

void __mock_eep_setup() {
  // Init every mix param to 2
  for (byte b = 0; b < CHANNEL_COUNT * PARAM_COUNT; b++) {
    __EEP_DATA[b + 7] = 2;
  }
}