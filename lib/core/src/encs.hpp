// Encoder input module. Heavily influenced by
// https://github.com/j-bellavance/CommonBusEncoders
// but much stupider.

#define BUS_A_PIN 23
#define BUS_B_PIN 25
#define BUS_S_PIN 49

const byte encs_pin[] = {34, 32, 30, 28, 26, 24, 22, 4, 5, 36, 38, 40, 42, 44, 46, 48, 7, 6};
int encs_lastA[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
int encs_lastS[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

void encs_setup(void) {
  pinMode(BUS_A_PIN, INPUT_PULLUP);
  pinMode(BUS_B_PIN, INPUT_PULLUP);
  pinMode(BUS_S_PIN, INPUT_PULLUP);

  for (byte enc = 0; enc < 18; enc++) {
    byte pin = encs_pin[enc];
    pinMode(pin, OUTPUT);
    digitalWrite(pin, HIGH);
  }
}

unsigned int encs_read(void) {
  byte pin;
  int busA;
  int busB;
  int busS;
  byte encAction = ENC_ACTION_NONE;
  byte enc;

  for (enc = 0; enc < 18; enc++) {
    pin = encs_pin[enc];
    digitalWrite(pin, LOW);                                      // Enable read for this pin

    busA = digitalRead(BUS_A_PIN);    // Read the status of the bus on pinA
    busS = digitalRead(BUS_S_PIN);
    if (!encs_lastA[enc] && (busA)) { // If pin A is RISING (was low and is high),
      busB = digitalRead(BUS_B_PIN);  // Read the status of the bus on pinB
      encAction = busB ? ENC_ACTION_INC : ENC_ACTION_DEC; // Get the direction
    } else if (encs_lastS[enc] != busS) { 
      encAction = busS ? ENC_ACTION_RELEASE : ENC_ACTION_PRESS;
    }
    encs_lastA[enc] = busA;           // Save busA & busS values for next read of this pin
    encs_lastS[enc] = busS;

    digitalWrite(pin, HIGH);          // Disabled read for this pin
    if (encAction) break; // This is the one
  }
  if (!encAction) return 0;
  return (enc << 8) + encAction;
}