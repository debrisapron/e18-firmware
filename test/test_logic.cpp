#include <Arduino.h>
#include <unity.h>

#include <shared.h>
#include <core.hpp>
#include <encs.hpp>
#include <gfx.hpp>
#include "mock_es9.hpp"
#include "mock_eep.hpp"

extern byte core_status;
extern byte core_paramIds[2];
extern E18State core_state;
extern char __es9_sysexMessages[100][30];
extern byte __es9_idx;

void setUp(void) {
    __es9_idx = 0; // Reset es9 mock
} 

void test_setup(void) {
    __eep_data = __EEP_EMPTY;
    TEST_ASSERT_EQUAL(0, core_status);
    core_setup();
    TEST_ASSERT_EQUAL(1, core_status);
    TEST_ASSERT_EQUAL_MESSAGE(0, core_state[0][0], "Vol 0 should be 0");
    TEST_ASSERT_EQUAL_MESSAGE(128, core_state[1][0], "Pan 0 should be 0");
    TEST_ASSERT_EQUAL_MESSAGE(1, core_paramIds[0], "Top row should be Pan");
    TEST_ASSERT_EQUAL_MESSAGE(0, core_paramIds[1], "Bottom row should be Vol");
}

void test_incrementValue(void) {
    for (int i = 0; i < 10; i++) {
        core_handleEnc(10, 1);
        delay(100);
    }
    // It should go up 2 for every click
    TEST_ASSERT_EQUAL_MESSAGE(20, core_state[0][0], "Vol 0 should be 22");
}

void test_decrementValue(void) {
    for (int i = 0; i < 12; i++) {
        core_handleEnc(10, -1);
        delay(100);
    }
    // It should go down 2 for every click, but not below zero
    TEST_ASSERT_EQUAL_MESSAGE(0, core_state[0][0], "Vol 0 should be 0");
}

void test_changeSelectedParam(void) {
    // Switch bottom row from volume to EQ type
    core_handleEnc(9, 1);
    delay(100);
    // It should skip pan
    TEST_ASSERT_EQUAL(2, core_paramIds[1]);
    core_handleEnc(9, -1);
    delay(100);
    TEST_ASSERT_EQUAL(0, core_paramIds[1]);

    // Switch top row from pan to EQ type
    core_handleEnc(0, 1);
    delay(100);
    TEST_ASSERT_EQUAL(2, core_paramIds[0]);
    core_handleEnc(0, -1);
    delay(100);
    TEST_ASSERT_EQUAL(1, core_paramIds[0]);
}

void test_incrementEqType(void) {
    core_handleEnc(0, 1);
    for (int i = 0; i < 10; i++) {
        core_handleEnc(1, 1);
        delay(100);
    }
    // It should go up 1 for every click, but not above 7
    TEST_ASSERT_EQUAL(7, core_state[2][0]);
    // Check sent sysex
    TEST_ASSERT_EQUAL_CHAR_ARRAY("39 00 00 07 00 00 00 00 00 00 00 00 00", __es9_sysexMessages[6], 38);
}

void test_setupWithLoadFromEEPROM(void) {
    __eep_data = __EEP_DATA;
    core_setup();
    TEST_ASSERT_EQUAL_MESSAGE(2, core_state[0][0], "Vol 0 should be 2");
    TEST_ASSERT_EQUAL_MESSAGE(2, core_state[1][0], "Pan 0 should be 2");
    TEST_ASSERT_EQUAL_MESSAGE(3, core_paramIds[0], "Top row should be EQ1 Freq");
    TEST_ASSERT_EQUAL_MESSAGE(2, core_paramIds[1], "Bottom row should be EQ1 Type");
    // Check sysex messages sent to ES9 for channel 0
    TEST_ASSERT_EQUAL_CHAR_ARRAY("34 00 01", __es9_sysexMessages[0], 8);
    TEST_ASSERT_EQUAL_CHAR_ARRAY("34 08 00", __es9_sysexMessages[1], 8);
    TEST_ASSERT_EQUAL_CHAR_ARRAY("34 10 00", __es9_sysexMessages[2], 8);
    TEST_ASSERT_EQUAL_CHAR_ARRAY("34 18 00", __es9_sysexMessages[3], 8);
}

// TODO Test eeprom save

void setup() {
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(test_setup);
    RUN_TEST(test_incrementValue);
    RUN_TEST(test_decrementValue);
    RUN_TEST(test_changeSelectedParam);
    RUN_TEST(test_incrementEqType);
    RUN_TEST(test_setupWithLoadFromEEPROM);
}

void loop() {
    UNITY_END();
}