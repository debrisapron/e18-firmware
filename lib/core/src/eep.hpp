#define EEP_VERSION 17
#define EEP_UID 0xB0, 0xF5, 0x66, 0x82
#define HEADER_LEN 5

const byte eep_header[HEADER_LEN] = {EEP_UID, EEP_VERSION};
bool eep_headerOk = false;

bool eep_load(Scene *scene, byte slot) {
  if (!eep_headerOk) {
    bool headerOk = true;
    for (byte h = 0; h < HEADER_LEN; h++) {
      headerOk = headerOk && (EEPROM.read(h) == eep_header[h]);
    }
    eep_headerOk = headerOk;
    if (!headerOk) return false;
  }

  EEPROM.get(HEADER_LEN + sizeof(Scene) * slot, *scene);
  return true;
}

void eep_save(Scene *scene, byte slot) {
  if (!eep_headerOk) {
    EEPROM.put(0, eep_header);
    eep_headerOk = true;
  }

  EEPROM.put(HEADER_LEN + sizeof(Scene) * slot, *scene);
}