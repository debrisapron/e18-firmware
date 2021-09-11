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

#define __2S 2, 2, 2, 2, 2, 2, 2, 2
byte __EEP_EMPTY[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
byte __EEP_DATA[] = {
  0xB0, 0xF5, 0x66, 0x82, EEP_VERSION,              // Header 
  3, 2,                                             // Selected params
  __2S, __2S, __2S, __2S, __2S, __2S, __2S, __2S    // Mix with every value = 2
};