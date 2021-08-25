#include "Arduino.h"
#include <CommonBusEncoders.h>

CommonBusEncoders encoders(25, 27, 51, 18);

void handleKnob(unsigned int code) {
  byte action = code % 10;
  byte knob = code * 0.1; // 1-based including param knob

  char buffer [12];
  sprintf(buffer, "%d %d %d", code, action, knob);
  Serial.println(buffer);
}

void initializeEncoder(int id, int pin) {
  encoders.addEncoder(id, 4, pin, 1, id * 10, (id * 10) + 9);
}

void initializeEncoders()
{
  encoders.resetChronoAfter(10);

  // Top row
  initializeEncoder(1, 36);
  initializeEncoder(2, 34);
  initializeEncoder(3, 32);
  initializeEncoder(4, 30);
  initializeEncoder(5, 28);
  initializeEncoder(6, 26);
  initializeEncoder(7, 24);
  initializeEncoder(8, 22);
  initializeEncoder(9, 23);
  
  // Bottom row
  initializeEncoder(10, 38);
  initializeEncoder(11, 40);
  initializeEncoder(12, 42);
  initializeEncoder(13, 44);
  initializeEncoder(14, 46);
  initializeEncoder(15, 48);
  initializeEncoder(16, 50);
  initializeEncoder(17, 52);
  initializeEncoder(18, 53);
}

void coreSetup(void) {
  Serial.begin(9600);
  initializeEncoders();
}

void coreLoop(void) {
  unsigned int code = encoders.readAll();
  if (code != 0) {
    handleKnob(code);
  }
}