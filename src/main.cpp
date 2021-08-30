#include <Arduino.h>
#include <core.hpp>
#include <knobs.hpp>

void setup() {
  knobs_setup();
  core_setup();
}

void loop() {
  unsigned int code = knobs_read();
  if (code != 0) {
    core_handleKnob(code);
  }
}