#include <Arduino.h>
#include <unity.h>
#include <core.hpp>
 
extern byte core_status;
extern byte core_param[];
extern byte core_state[][8];

// void __log(const char* name, const char* value) {
//     char msg[100] = "";
//     strcat(msg, name);
//     strcat(msg, ": ");
//     strcat(msg, value);
//     TEST_MESSAGE(msg);
// }

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
    // It should go up 2 for every click
    TEST_ASSERT_EQUAL(20, core_state[0][0]);
}

void test_handleKnob_decrement(void) {
    for (int i = 0; i < 12; i++) {
        core_handleKnob(20);
        delay(100);
    }
    // It should go down 2 for every click, but not below zero
    TEST_ASSERT_EQUAL(0, core_state[0][0]);
}

void test_handleKnob_param(void) {
    core_handleKnob(11);
    delay(100);
    // It should skip the other row's param
    TEST_ASSERT_EQUAL(2, core_param[0]);
    core_handleKnob(10);
    delay(100);
    TEST_ASSERT_EQUAL(0, core_param[0]);

    // Bottom row
    core_handleKnob(101);
    delay(100);
    TEST_ASSERT_EQUAL(2, core_param[1]);
    core_handleKnob(100);
    delay(100);
    TEST_ASSERT_EQUAL(1, core_param[1]);
}

void test_handleKnob_eqType(void) {
    core_handleKnob(101);
    for (int i = 0; i < 10; i++) {
        core_handleKnob(111);
        delay(100);
    }
    // It should go up 1 for every click, but not above 7
    TEST_ASSERT_EQUAL(7, core_state[2][0]);
}

void setup() {
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(test_start);
    RUN_TEST(test_handleKnob_increment);
    RUN_TEST(test_handleKnob_decrement);
    RUN_TEST(test_handleKnob_param);
    RUN_TEST(test_handleKnob_eqType);
}

void loop() {
    UNITY_END();
}