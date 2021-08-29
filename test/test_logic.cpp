#include <Arduino.h>
#include <unity.h>
#include <core.hpp>

extern byte core_status;
extern byte core_param[];
extern int core_state[][8];

void test_start(void) {
    TEST_ASSERT_EQUAL(0, core_status);
    core_setup();
    TEST_ASSERT_EQUAL(1, core_status);
    TEST_ASSERT_EQUAL(0, core_state[0][0]);
    TEST_ASSERT_EQUAL(0, core_param[0]);
}

void test_handleKnob_increment(void) {
    for (int i = 0; i < 10; i++) {
        core_handleKnob(21);
        delay(100);
    }
    TEST_ASSERT_EQUAL(10, core_state[0][0]);
}

void test_handleKnob_decrement(void) {
    for (int i = 0; i < 10; i++) {
        core_handleKnob(20);
        delay(100);
    }
    TEST_ASSERT_EQUAL(0, core_state[0][0]);
}

void test_handleKnob_param(void) {
    core_handleKnob(11);
    delay(100);
    TEST_ASSERT_EQUAL(1, core_param[0]);
    core_handleKnob(10);
    delay(100);
    TEST_ASSERT_EQUAL(0, core_param[0]);
}

void test_handleKnob_pan(void) {
    core_handleKnob(11);
    delay(100);
    for (int i = 0; i < 5; i++) {
        core_handleKnob(91);
        delay(100);
    }
    TEST_ASSERT_EQUAL(5, core_state[1][7]);
    for (int i = 0; i < 10; i++) {
        core_handleKnob(90);
        delay(100);
    }
    TEST_ASSERT_EQUAL(-5, core_state[1][7]);
    for (int i = 0; i < 5; i++) {
        core_handleKnob(91);
        delay(100);
    }
    TEST_ASSERT_EQUAL(0, core_state[1][7]);
}

void test_handleKnob_bottom(void) {
    core_handleKnob(101);
    delay(100);
    TEST_ASSERT_EQUAL(1, core_param[1]);
    for (int i = 0; i < 5; i++) {
        core_handleKnob(111);
        delay(100);
    }
    TEST_ASSERT_EQUAL(5, core_state[1][0]);
}

void setup() {
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(test_start);
    RUN_TEST(test_handleKnob_increment);
    RUN_TEST(test_handleKnob_decrement);
    RUN_TEST(test_handleKnob_param);
    RUN_TEST(test_handleKnob_pan);
    RUN_TEST(test_handleKnob_bottom);
}

void loop() {
    UNITY_END();
}