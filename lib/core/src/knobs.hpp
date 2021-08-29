#include <CommonBusEncoders.h>

CommonBusEncoders knobs_encoders(23, 25, 49, 18);

// void handleKnob(unsigned int code) {
//   char buffer [5];
//   sprintf(buffer, "%d", code);
//   Serial.println(buffer);
// }

void knobs_initializeEncoder(int id, int pin) {
  knobs_encoders.addEncoder(id, 4, pin, 1, id * 10, (id * 10) + 9);
}

void knobs_initializeEncoders()
{
  knobs_encoders.resetChronoAfter(10);

  // Top row
  knobs_initializeEncoder(1, 34);
  knobs_initializeEncoder(2, 32);
  knobs_initializeEncoder(3, 30);
  knobs_initializeEncoder(4, 28);
  knobs_initializeEncoder(5, 26);
  knobs_initializeEncoder(6, 24);
  knobs_initializeEncoder(7, 22);
  knobs_initializeEncoder(8, 4);
  knobs_initializeEncoder(9, 5);
  
  // Bottom row
  knobs_initializeEncoder(10, 36);
  knobs_initializeEncoder(11, 38);
  knobs_initializeEncoder(12, 40);
  knobs_initializeEncoder(13, 42);
  knobs_initializeEncoder(14, 44);
  knobs_initializeEncoder(15, 46);
  knobs_initializeEncoder(16, 48);
  knobs_initializeEncoder(17, 7);
  knobs_initializeEncoder(18, 6);
}

void knobs_setup(void) {
  knobs_initializeEncoders();
}

int knobs_read(void) {
  return knobs_encoders.readAll();
}