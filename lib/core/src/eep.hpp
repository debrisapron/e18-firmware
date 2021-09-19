#define EEP_VERSION 9
#define EEP_UID 0xB0, 0xF5, 0x66, 0x82
#define HEADER_LEN 5

const byte eep_header[HEADER_LEN] = {EEP_UID, EEP_VERSION};

bool eep_load(Scene *scene, Scene sceneSlots[8]) {
  bool headerOk = true;
  for (byte h = 0; h < HEADER_LEN; h++) {
    headerOk = headerOk && (EEPROM.read(h) == eep_header[h]);
  }
  if (!headerOk) return false;

  EEPROM.get(HEADER_LEN, *scene);
  EEPROM.get(HEADER_LEN + sizeof(Scene), sceneSlots);
  return true;
}

void eep_save(Scene *scene, Scene sceneSlots[8]) {
  EEPROM.put(0, eep_header);

  EEPROM.put(HEADER_LEN, *scene);
  EEPROM.put(HEADER_LEN + sizeof(Scene), sceneSlots);
}