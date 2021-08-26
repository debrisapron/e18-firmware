#include "Arduino.h"
#include <CommonBusEncoders.h>

CommonBusEncoders encoders(23, 25, 49, 18);

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
  initializeEncoder(1, 34);
  initializeEncoder(2, 32);
  initializeEncoder(3, 30);
  initializeEncoder(4, 28);
  initializeEncoder(5, 26);
  initializeEncoder(6, 24);
  initializeEncoder(7, 22);
  initializeEncoder(8, 4);
  initializeEncoder(9, 5);
  
  // Bottom row
  initializeEncoder(10, 36);
  initializeEncoder(11, 38);
  initializeEncoder(12, 40);
  initializeEncoder(13, 42);
  initializeEncoder(14, 44);
  initializeEncoder(15, 46);
  initializeEncoder(16, 48);
  initializeEncoder(17, 7);
  initializeEncoder(18, 6);
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