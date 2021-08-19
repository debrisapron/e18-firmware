#include <Arduino.h>
#include <unity.h>
#include <core.hpp>

// void setUp(void) {
//     state = 0;
// }

extern byte status;
extern int state[];

void test_start(void) {
    TEST_ASSERT_EQUAL(0, status);
    start();
    TEST_ASSERT_EQUAL(1, status);
    TEST_ASSERT_EQUAL(0, state[0]);
}

void test_handleKnob_increment(void) {
    for (int i = 0; i < 10; i++) {
        handleKnob(201);
        delay(100);
    }
    TEST_ASSERT_EQUAL(10, state[1]);
}

void test_handleKnob_decrement(void) {
    for (int i = 0; i < 10; i++) {
        handleKnob(202);
        delay(100);
    }
    TEST_ASSERT_EQUAL(0, state[1]);
}

void setup() {
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(test_start);
    RUN_TEST(test_handleKnob_increment);
    RUN_TEST(test_handleKnob_decrement);
}

void loop() {
    UNITY_END();
}