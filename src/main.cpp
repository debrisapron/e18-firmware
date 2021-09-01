#include <Arduino.h>
#include <core.hpp>
#include <encs.hpp>

void setup() {
  encs_setup();
  core_setup();
}

void loop() {
  encs_read();
  if (encs_newIndex > -1) {
    core_handleEnc(encs_newIndex, encs_newAction);
  }
}