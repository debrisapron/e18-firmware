#include <Arduino.h>
#include <EEPROM.h>
#include <MIDI.h>

#include <shared.h>
#include <core.hpp>
#include <encs.hpp>
#include <gfx.hpp>
#include <es9.hpp>
#include <eep.hpp>

void setup(void) {
  core_setup();
}

void loop(void) {
  core_loop();
}