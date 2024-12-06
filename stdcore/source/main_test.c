#include "datetime.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

void test_dt_now() {
    bool valid;
    dt_t now = dt_now(&valid);
    assert(valid == true);
    printf("test_dt_now passed\n");
}

void test_dt_create() {
    bool valid;
    dt_t dt = dt_create(2024, 12, 5, 14, 30, 0, 500, &valid);
    assert(valid == true);
    assert(dt.year == 2024);
    assert(dt.month == 12);
    assert(dt.day == 5);
    assert(dt.hour == 14);
    assert(dt.minute == 30);
    assert(dt.second == 0);
    assert(dt.millisecond == 500);
    printf("test_dt_create passed\n");
}

void test_dt_isValid() {
    dt_t dt = { 2024, 2, 29, 12, 0, 0, 0 };
    assert(dt_isValid(&dt) == true);
    dt = (dt_t){ 2023, 2, 29, 12, 0, 0, 0 };
    assert(dt_isValid(&dt) == false);
    printf("test_dt_isValid passed\n");
}

void test_dt_parse() {
    bool valid;
    dt_t dt = dt_parse("2024-12-05T14:30:00.500Z", &valid);
    assert(valid == true);
    assert(dt.year == 2024);
    assert(dt.month == 12);
    assert(dt.day == 5);
    assert(dt.hour == 14);
    assert(dt.minute == 30);
    assert(dt.second == 0);
    assert(dt.millisecond == 500);
    printf("test_dt_parse passed\n");
}

void test_dt_toISOString() {
    bool valid;
    dt_t dt = dt_create(2024, 12, 5, 14, 30, 0, 500, &valid);
    char buffer[26];
    dt_toISOString(&dt, buffer, sizeof(buffer), &valid);
    assert(valid == true);
    assert(strcmp(buffer, "2024-12-05T14:30:00.500Z") == 0);
    printf("test_dt_toISOString passed\n");
}

void test_dt_toUnixTimestamp() {
    bool valid;
    dt_t dt = dt_create(1970, 1, 1, 0, 0, 0, 0, &valid);
    int64_t timestamp = dt_toUnixTimestamp(&dt, &valid);
    assert(valid == true);
    assert(timestamp == 0);
    printf("test_dt_toUnixTimestamp passed\n");
}

void test_dt_fromUnixTimestamp() {
    bool valid;
    dt_t dt = dt_fromUnixTimestamp(0, &valid);
    assert(valid == true);
    assert(dt.year == 1970);
    assert(dt.month == 1);
    assert(dt.day == 1);
    assert(dt.hour == 0);
    assert(dt.minute == 0);
    assert(dt.second == 0);
    assert(dt.millisecond == 0);
    printf("test_dt_fromUnixTimestamp passed\n");
}

void test_dt_compare() {
    bool valid;
    dt_t dt1 = dt_create(2024, 12, 5, 14, 30, 0, 500, &valid);
    dt_t dt2 = dt_create(2024, 12, 5, 14, 30, 0, 600, &valid);
    assert(dt_compare(&dt1, &dt2) == -1);
    assert(dt_compare(&dt2, &dt1) == 1);
    assert(dt_compare(&dt1, &dt1) == 0);
    printf("test_dt_compare passed\n");
}

void test_dt_add() {
    bool valid;
    dt_t dt = dt_create(2024, 12, 5, 14, 30, 0, 500, &valid);
    dt = dt_add(&dt, 1, 1, 0, 0, 0, &valid);
    assert(valid == true);
    assert(dt.year == 2024);
    assert(dt.month == 12);
    assert(dt.day == 6);
    assert(dt.hour == 15);
    assert(dt.minute == 30);
    printf("test_dt_add passed\n");
}

void test_dt_subtract() {
    bool valid;
    dt_t dt = dt_create(2024, 12, 5, 14, 30, 0, 500, &valid);
    dt = dt_subtract(&dt, 1, 1, 0, 0, 0, &valid);
    assert(valid == true);
    assert(dt.year == 2024);
    assert(dt.month == 12);
    assert(dt.day == 4);
    assert(dt.hour == 13);
    assert(dt.minute == 30);
    printf("test_dt_subtract passed\n");
}

void test_dt_getDayOfWeek() {
    bool valid;
    dt_t dt = dt_create(2024, 12, 5, 0, 0, 0, 0, &valid);
    int dayOfWeek = dt_getDayOfWeek(&dt);
    assert(dayOfWeek == 4);  // 2024-12-05 is a Thursday
    printf("test_dt_getDayOfWeek passed\n");
}

void test_dt_format() {
    bool valid;
    dt_t dt = dt_create(2024, 12, 5, 14, 30, 0, 500, &valid);
    char buffer[100];
    dt_format(&dt, "%Y-%m-%d %H:%M:%S", buffer, sizeof(buffer), &valid);
    assert(valid == true);
    assert(strcmp(buffer, "2024-12-05 14:30:00") == 0);
    printf("test_dt_format passed\n");
}

void test_dt_getYear() {
    bool valid;
    dt_t dt = dt_create(2024, 12, 5, 14, 30, 0, 500, &valid);
    assert(dt_getYear(&dt) == 2024);
    printf("test_dt_getYear passed\n");
}

void test_dt_getMonth() {
    bool valid;
    dt_t dt = dt_create(2024, 12, 5, 14, 30, 0, 500, &valid);
    assert(dt_getMonth(&dt) == 12);
    printf("test_dt_getMonth passed\n");
}

void test_dt_getDay() {
    bool valid;
    dt_t dt = dt_create(2024, 12, 5, 14, 30, 0, 500, &valid);
    assert(dt_getDay(&dt) == 5);
    printf("test_dt_getDay passed\n");
}

void test_dt_getHour() {
    bool valid;
    dt_t dt = dt_create(2024, 12, 5, 14, 30, 0, 500, &valid);
    assert(dt_getHour(&dt) == 14);
    printf("test_dt_getHour passed\n");
}

void test_dt_getMinute() {
    bool valid;
    dt_t dt = dt_create(2024, 12, 5, 14, 30, 0, 500, &valid);
    assert(dt_getMinute(&dt) == 30);
    printf("test_dt_getMinute passed\n");
}

void test_dt_getSecond() {
    bool valid;
    dt_t dt = dt_create(2024, 12, 5, 14, 30, 0, 500, &valid);
    assert(dt_getSecond(&dt) == 0);
    printf("test_dt_getSecond passed\n");
}

void test_dt_getMillisecond() {
    bool valid;
    dt_t dt = dt_create(2024, 12, 5, 14, 30, 0, 500, &valid);
    assert(dt_getMillisecond(&dt) == 500);
    printf("test_dt_getMillisecond passed\n");
}

void test_dt_diff() {
    bool valid;
    dt_t dt1 = dt_create(2024, 12, 5, 14, 30, 0, 500, &valid);
    dt_t dt2 = dt_create(2024, 12, 5, 14, 30, 1, 0, &valid);
    int64_t diff = dt_diff(&dt1, &dt2, &valid);
    assert(valid == true);
    assert(diff == -500);  // Difference in milliseconds
    printf("test_dt_diff passed\n");
}

int __main__() {
    test_dt_now();
    test_dt_create();
    test_dt_isValid();
    test_dt_parse();
    test_dt_toISOString();
    test_dt_toUnixTimestamp();
    test_dt_fromUnixTimestamp();
    test_dt_compare();
    test_dt_add();
    test_dt_subtract();
    test_dt_getDayOfWeek();
    test_dt_format();
    test_dt_getYear();
    test_dt_getMonth();
    test_dt_getDay();
    test_dt_getHour();
    test_dt_getMinute();
    test_dt_getSecond();
    test_dt_getMillisecond();
    test_dt_diff();
    printf("All tests passed!\n");
    return 0;
}
