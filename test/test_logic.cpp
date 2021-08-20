#include <Arduino.h>
#include <unity.h>
#include <core.hpp>

// void setUp(void) {
//     state = 0;
// }

extern byte status;
extern byte param;
extern int state[][8];

void test_start(void) {
    TEST_ASSERT_EQUAL(0, status);
    start();
    TEST_ASSERT_EQUAL(1, status);
    TEST_ASSERT_EQUAL(0, state[0][0]);
    TEST_ASSERT_EQUAL(0, param);
}

void test_handleKnob_increment(void) {
    for (int i = 0; i < 10; i++) {
        handleKnob(201);
        delay(100);
    }
    TEST_ASSERT_EQUAL(10, state[0][0]);
}

void test_handleKnob_decrement(void) {
    for (int i = 0; i < 10; i++) {
        handleKnob(202);
        delay(100);
    }
    TEST_ASSERT_EQUAL(0, state[0][0]);
}

void test_handleKnob_param(void) {
    handleKnob(101);
    delay(100);
    TEST_ASSERT_EQUAL(1, param);
    handleKnob(102);
    delay(100);
    TEST_ASSERT_EQUAL(0, param);
}

void test_handleKnob_pan(void) {
    handleKnob(101);
    delay(100);
    for (int i = 0; i < 5; i++) {
        handleKnob(301);
        delay(100);
    }
    TEST_ASSERT_EQUAL(5, state[1][1]);
    for (int i = 0; i < 10; i++) {
        handleKnob(302);
        delay(100);
    }
    TEST_ASSERT_EQUAL(-5, state[1][1]);
    for (int i = 0; i < 5; i++) {
        handleKnob(301);
        delay(100);
    }
    TEST_ASSERT_EQUAL(0, state[1][1]);
}

void setup() {
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(test_start);
    RUN_TEST(test_handleKnob_increment);
    RUN_TEST(test_handleKnob_decrement);
    RUN_TEST(test_handleKnob_param);
    RUN_TEST(test_handleKnob_pan);
}

void loop() {
    UNITY_END();
}