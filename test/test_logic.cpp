#include <Arduino.h>
#include <unity.h>
#include <core.hpp>

extern byte status;
extern byte param[];
extern int state[][8];

void test_start(void) {
    TEST_ASSERT_EQUAL(0, status);
    coreSetup();
    TEST_ASSERT_EQUAL(1, status);
    TEST_ASSERT_EQUAL(0, state[0][0]);
    TEST_ASSERT_EQUAL(0, param[0]);
}

void test_handleKnob_increment(void) {
    for (int i = 0; i < 10; i++) {
        handleKnob(21);
        delay(100);
    }
    TEST_ASSERT_EQUAL(10, state[0][0]);
}

void test_handleKnob_decrement(void) {
    for (int i = 0; i < 10; i++) {
        handleKnob(20);
        delay(100);
    }
    TEST_ASSERT_EQUAL(0, state[0][0]);
}

void test_handleKnob_param(void) {
    handleKnob(11);
    delay(100);
    TEST_ASSERT_EQUAL(1, param[0]);
    handleKnob(10);
    delay(100);
    TEST_ASSERT_EQUAL(0, param[0]);
}

void test_handleKnob_pan(void) {
    handleKnob(11);
    delay(100);
    for (int i = 0; i < 5; i++) {
        handleKnob(91);
        delay(100);
    }
    TEST_ASSERT_EQUAL(5, state[1][7]);
    for (int i = 0; i < 10; i++) {
        handleKnob(90);
        delay(100);
    }
    TEST_ASSERT_EQUAL(-5, state[1][7]);
    for (int i = 0; i < 5; i++) {
        handleKnob(91);
        delay(100);
    }
    TEST_ASSERT_EQUAL(0, state[1][7]);
}

void test_handleKnob_bottom(void) {
    handleKnob(101);
    delay(100);
    TEST_ASSERT_EQUAL(1, param[1]);
    for (int i = 0; i < 5; i++) {
        handleKnob(111);
        delay(100);
    }
    TEST_ASSERT_EQUAL(5, state[1][0]);
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