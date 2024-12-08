//
// Created by praisethemoon on 01.01.24.
//
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <inttypes.h>

#include "datetime.h"
#include "vendor/yy.h"
#include "../../source/core.h"
#include "../../source/api/typev_api.h"
#include "../../source/errors/errors.h"

#define MAX_NUM_STR_SIZE 64

// Convert floating point types to string
void double_to_str(TypeV_Core *core) {
    char buffer[MAX_NUM_STR_SIZE] = {0};
    double value = typev_api_stack_pop_f64(core);

    char* ending = yy_double_to_string(value, buffer);

    long int length = ending - buffer + 1;

    TypeV_Array* str = (TypeV_Array*)core_array_alloc(core, 0, length, 1);
    memcpy(str->data, buffer, length);

    typev_api_return_ptr(core, (size_t)str);
}

// Convert floating point types to string
void float_to_str(TypeV_Core *core) {
    char buffer[MAX_NUM_STR_SIZE] = {0};
    float value = typev_api_stack_pop_f32(core);
    double value_64 = (double)value;

    char* ending = yy_double_to_string(value_64, buffer);

    long int size = ending - buffer + 1;

    TypeV_Array* str = (TypeV_Array*)core_array_alloc(core, 0, size, 1);
    memcpy(str->data, buffer, size);

    typev_api_return_ptr(core, (size_t)str);
}

// Convert boolean type to string
void bool_to_str(TypeV_Core *core) {
    char buffer[MAX_NUM_STR_SIZE] = {0};
    uint8_t value = typev_api_stack_pop_u8(core);

    int size = snprintf(buffer, MAX_NUM_STR_SIZE, "%s", value?"true":"false");
    TypeV_Array* str = (TypeV_Array*)core_array_alloc(core, 0, size, 1);
    memcpy(str->data, buffer, size);

    typev_api_return_ptr(core, (size_t)str);
}

void uint8_to_str(TypeV_Core *core) {
    char buffer[MAX_NUM_STR_SIZE] = {0};
    uint8_t value = typev_api_stack_pop_u8(core);

    int size = snprintf(buffer, MAX_NUM_STR_SIZE, "%" PRIu8, value);
    TypeV_Array* str = (TypeV_Array*)core_array_alloc(core, 0, size, 1);
    memcpy(str->data, buffer, size);

    typev_api_return_ptr(core, (size_t)str);
}

void int8_to_str(TypeV_Core *core) {

    char buffer[MAX_NUM_STR_SIZE] = {0};
    int8_t value = typev_api_stack_pop_i8(core);

    int size = snprintf(buffer, MAX_NUM_STR_SIZE, "%" PRId8, value);
    TypeV_Array* str = (TypeV_Array*)core_array_alloc(core, 0, size, 1);
    memcpy(str->data, buffer, size);

    typev_api_return_ptr(core, (size_t)str);
}


void uint16_to_str(TypeV_Core *core) {
    char buffer[MAX_NUM_STR_SIZE] = {0};
    uint16_t value = typev_api_stack_pop_u16(core);

    int size = snprintf(buffer, MAX_NUM_STR_SIZE, "%" PRIu16, value);
    TypeV_Array* str = (TypeV_Array*)core_array_alloc(core, 0, size, 1);
    memcpy(str->data, buffer, size);

    typev_api_return_ptr(core, (size_t)str);
}

void int16_to_str(TypeV_Core *core) {

    char buffer[MAX_NUM_STR_SIZE] = {0};
    int16_t value = typev_api_stack_pop_i16(core);

    int size = snprintf(buffer, MAX_NUM_STR_SIZE, "%" PRId16, value);
    TypeV_Array* str = (TypeV_Array*)core_array_alloc(core, 0, size, 1);
    memcpy(str->data, buffer, size);

    typev_api_return_ptr(core, (size_t)str);
}


void uint32_to_str(TypeV_Core *core) {
    char buffer[10] = {0};
    uint32_t value = typev_api_stack_pop_u32(core);

    char* end = itoa_u32_yy(value, buffer);
    long int size = end - buffer + 1;

    TypeV_Array* str = (TypeV_Array*)core_array_alloc(core, 0, size, 1);
    memcpy(str->data, buffer, size);

    typev_api_return_ptr(core, (size_t)str);
}

void int32_to_str(TypeV_Core *core) {

    char buffer[11] = {0};
    int32_t value = typev_api_stack_pop_i32(core);

    char* end = itoa_i32_yy(value, buffer);
    long int size = end - buffer + 1;

    TypeV_Array* str = (TypeV_Array*)core_array_alloc(core, 0, size, 1);
    memcpy(str->data, buffer, size);

    typev_api_return_ptr(core, (size_t)str);
}


void uint64_to_str(TypeV_Core *core) {
    char buffer[20] = {0};
    uint64_t value = typev_api_stack_pop_u64(core);

    char* end = itoa_u64_yy(value, buffer);
    long int size = end - buffer + 1;

    TypeV_Array* str = (TypeV_Array*)core_array_alloc(core, 0, size, 1);
    memcpy(str->data, buffer, size);

    typev_api_return_ptr(core, (size_t)str);
}

void int64_to_str(TypeV_Core *core) {
    char buffer[20] = {0};
    int64_t value = typev_api_stack_pop_i64(core);

    char* end = itoa_i64_yy(value, buffer);
    long int size = end - buffer + 1;

    TypeV_Array* str = (TypeV_Array*)core_array_alloc(core, 0, size, 1);
    memcpy(str->data, buffer, size);

    typev_api_return_ptr(core, (size_t)str);

    typev_api_return_ptr(core, (size_t)str);
}

void string_append_f64(TypeV_Core* core){
    TypeV_Array* str = (TypeV_Array*)typev_api_stack_pop_ptr(core);
    double value = typev_api_stack_pop_f64(core);
    uint32_t pos = typev_api_stack_pop_u32(core);

    // check if the array has enough capacity starting from pos
    if(str->length < pos + MAX_NUM_STR_SIZE){
        // realloc the array
        core_array_extend(core, (size_t)str, pos+MAX_NUM_STR_SIZE);
    }

    char* ending = yy_double_to_string(value, (char*)str->data+pos);

    if(ending == NULL){
        typev_api_core_panic(core, 1, "Failed to convert double to string\n");
        return;
    }

    ptrdiff_t gap = ending - ((char*)str->data+pos);

    if (gap < 0 || gap > UINT32_MAX) {
        typev_api_core_panic(core, 2, "Gap value is out of range for uint32_t\n");
        return;
    }

    typev_api_return_u32(core, gap);
}

#define MAX_FLOAT_STR_SIZE 20

void string_append_f32(TypeV_Core* core){
    TypeV_Array* str = (TypeV_Array*)typev_api_stack_pop_ptr(core);
    float value_f32 = typev_api_stack_pop_f32(core);
    uint32_t pos = typev_api_stack_pop_u32(core);

    double value = (double)value_f32;

    // check if the array has enough capacity starting from pos
    if(str->length < pos + MAX_NUM_STR_SIZE){
        // realloc the array
        core_array_extend(core, (size_t)str, pos+MAX_NUM_STR_SIZE);
    }

    char* ending = yy_double_to_string(value, (char*)str->data+pos);

    if(ending == NULL){
        typev_api_core_panic(core, 1, "Failed to convert double to string\n");
        return;
    }

    ptrdiff_t gap = ending - ((char*)str->data+pos);

    if (gap < 0 || gap > UINT32_MAX) {
        typev_api_core_panic(core, 2, "Gap value is out of range for uint32_t\n");
        return;
    }

    typev_api_return_u32(core, gap);
}

#define MAX_UINT64_STR_SIZE 20

void string_append_u64(TypeV_Core* core){
    TypeV_Array* str = (TypeV_Array*)typev_api_stack_pop_ptr(core);
    uint64_t value = typev_api_stack_pop_u64(core);
    uint32_t pos = typev_api_stack_pop_u32(core);

    // check if the array has enough capacity starting from pos
    if(str->length < pos + MAX_UINT64_STR_SIZE){
        // realloc the array
        core_array_extend(core, (size_t)str, pos+MAX_UINT64_STR_SIZE);
    }

    // convert the value to string
    char* ending = itoa_u64_yy(value, (char*)str->data+pos);
    if(ending == NULL){
        typev_api_core_panic(core, 1, "Failed to convert uint64_t to string\n");
        return;
    }
    ptrdiff_t gap = ending - ((char*)str->data+pos);

    if (gap < 0 || gap > UINT32_MAX) {
        typev_api_core_panic(core, 2, "Gap value is out of range for uint32_t\n");
        return;
    }

    typev_api_return_u32(core, gap);
}

#define MAX_INT64_STR_SIZE 20
void string_append_i64(TypeV_Core* core){
    TypeV_Array* str = (TypeV_Array*)typev_api_stack_pop_ptr(core);
    int64_t value = typev_api_stack_pop_i64(core);
    uint32_t pos = typev_api_stack_pop_u32(core);

    // check if the array has enough capacity starting from pos
    if(str->length < pos + MAX_INT64_STR_SIZE){
        // realloc the array
        core_array_extend(core, (size_t)str, pos+MAX_INT64_STR_SIZE);
    }

    // convert the value to string
    char* ending = itoa_i64_yy(value, (char*)str->data+pos);
    if(ending == NULL){
        typev_api_core_panic(core, 1, "Failed to convert int64_t to string\n");
        return;
    }

    ptrdiff_t gap = ending - ((char*)str->data+pos);

    if (gap < 0 || gap > UINT32_MAX) {
        typev_api_core_panic(core, 2, "Gap value is out of range for uint32_t\n");
        return;
    }

    typev_api_return_u32(core, gap);
}

#define MAX_UINT32_STR_SIZE 12
void string_append_u32(TypeV_Core* core){
    TypeV_Array* str = (TypeV_Array*)typev_api_stack_pop_ptr(core);
    uint32_t value = typev_api_stack_pop_u32(core);
    uint32_t pos = typev_api_stack_pop_u32(core);

    // check if the array has enough capacity starting from pos
    if(str->length < pos + MAX_UINT32_STR_SIZE){
        // realloc the array
        core_array_extend(core, (size_t)str, pos+MAX_UINT32_STR_SIZE);
    }

    // convert the value to string
    char* ending = itoa_u32_yy(value, (char*)str->data+pos);
    ptrdiff_t gap = ending - ((char*)str->data+pos);

    if (gap < 0 || gap > UINT32_MAX) {
        typev_api_core_panic(core, 2, "Gap value is out of range for uint32_t\n");
        return;
    }

    typev_api_return_u32(core, gap);
}

#define MAX_INT32_STR_SIZE 12
void string_append_i32(TypeV_Core* core){
    TypeV_Array* str = (TypeV_Array*)typev_api_stack_pop_ptr(core);
    int32_t value = typev_api_stack_pop_i32(core);
    uint32_t pos = typev_api_stack_pop_u32(core);

    // check if the array has enough capacity starting from pos
    if(str->length < pos + MAX_INT32_STR_SIZE){
        // realloc the array
        core_array_extend(core, (size_t)str, pos+MAX_INT32_STR_SIZE);
    }

    char* ending = itoa_i32_yy(value, (char*)str->data+pos);
    uint32_t gap = ending - ((char*)str->data+pos);

    typev_api_return_u32(core, gap);
}

#define MAX_UINT16_STR_SIZE 6

void string_append_u16(TypeV_Core* core){
    TypeV_Array* str = (TypeV_Array*)typev_api_stack_pop_ptr(core);
    uint16_t value = typev_api_stack_pop_u16(core);
    uint32_t pos = typev_api_stack_pop_u32(core);

    // check if the array has enough capacity starting from pos
    if(str->length < pos + MAX_UINT16_STR_SIZE){
        // realloc the array
        core_array_extend(core, (size_t)str, pos+MAX_UINT16_STR_SIZE);
    }

    // convert the value to string
    int32_t gap = snprintf((char*)str->data+pos, MAX_UINT16_STR_SIZE, "%" PRIu16, value);
    if(gap < 0){
        core_panic(core, RT_ERROR_CUSTOM, "Failed to convert uint16_t to string, snprintf returned %d\n", gap);
    }

    typev_api_return_u32(core, gap);
}

#define MAX_INT16_STR_SIZE 7
void string_append_i16(TypeV_Core* core){
    TypeV_Array* str = (TypeV_Array*)typev_api_stack_pop_ptr(core);
    int16_t value = typev_api_stack_pop_i16(core);
    uint32_t pos = typev_api_stack_pop_u32(core);

    // check if the array has enough capacity starting from pos
    if(str->length < pos + MAX_INT16_STR_SIZE){
        // realloc the array
        core_array_extend(core, (size_t)str, pos+MAX_INT16_STR_SIZE);
    }

    // convert the value to string
    int32_t gap = snprintf((char*)str->data+pos, MAX_INT16_STR_SIZE, "%" PRId16, value);
    if(gap < 0){
        core_panic(core, RT_ERROR_CUSTOM, "Failed to convert int16_t to string, snprintf returned %d\n", gap);
    }

    typev_api_return_u32(core, gap);
}

#define MAX_UINT8_STR_SIZE 4
void string_append_u8(TypeV_Core* core){
    TypeV_Array* str = (TypeV_Array*)typev_api_stack_pop_ptr(core);
    uint8_t value = typev_api_stack_pop_u8(core);
    uint32_t pos = typev_api_stack_pop_u32(core);

    // check if the array has enough capacity starting from pos
    if(str->length < pos + MAX_UINT8_STR_SIZE){
        // realloc the array
        core_array_extend(core, (size_t)str, pos+MAX_UINT8_STR_SIZE);
    }

    // convert the value to string
    int32_t gap = snprintf((char*)str->data+pos, MAX_UINT8_STR_SIZE, "%" PRIu8, value);
    if(gap < 0){
        core_panic(core, RT_ERROR_CUSTOM, "Failed to convert uint8_t to string, snprintf returned %d\n", gap);
    }

    typev_api_return_u32(core, gap);
}

#define MAX_INT8_STR_SIZE 5
void string_append_i8(TypeV_Core* core){
    TypeV_Array* str = (TypeV_Array*)typev_api_stack_pop_ptr(core);
    int8_t value = typev_api_stack_pop_i8(core);
    uint32_t pos = typev_api_stack_pop_u32(core);

    // check if the array has enough capacity starting from pos
    if(str->length < pos + MAX_INT8_STR_SIZE){
        // realloc the array
        core_array_extend(core, (size_t)str, pos+MAX_INT8_STR_SIZE);
    }

    // convert the value to string
    int32_t gap = snprintf((char*)str->data+pos, MAX_INT8_STR_SIZE, "%" PRId8, value);
    if(gap < 0){
        core_panic(core, RT_ERROR_CUSTOM, "Failed to convert int8_t to string, snprintf returned %d\n", gap);
    }

    typev_api_return_u32(core, gap);
}

void string_append_bool(TypeV_Core* core){
    TypeV_Array* str = (TypeV_Array*)typev_api_stack_pop_ptr(core);
    uint8_t value = typev_api_stack_pop_u8(core);
    uint32_t pos = typev_api_stack_pop_u32(core);

    char* bool_str = value?"true":"false";

    if(str->length < pos + 6){
        core_array_extend(core, (size_t)str, pos+6);
    }

    int32_t gap = snprintf((char*)str->data+pos, 6, "%s", bool_str);
    if(gap < 0){
        core_panic(core, RT_ERROR_CUSTOM, "Failed to convert bool to string, snprintf returned %d\n", gap);
    }

    typev_api_return_u32(core, gap);
}

void string_toF64(TypeV_Core* core){
    TypeV_Array* str = (TypeV_Array*)typev_api_stack_pop_ptr(core);

    char* ptr;
    double value = yy_string_to_double((char*)str->data, &ptr);

    if (ptr == (char*)str->data) {
        typev_api_core_panic(core, RT_ERROR_CUSTOM, "Failed to convert string to float\n");
        return;
    }

    typev_api_return_f64(core, value);
}

void string_toF32(TypeV_Core* core){
    TypeV_Array* str = (TypeV_Array*)typev_api_stack_pop_ptr(core);
    char* ptr;
    double value = yy_string_to_double((char*)str->data, &ptr);
    if(ptr == (char*)str->data){
        typev_api_core_panic(core, RT_ERROR_CUSTOM, "Failed to convert string to float\n");
        return;
    }

    typev_api_return_f32(core, (float)value);
}

void string_toBool(TypeV_Core* core){
    TypeV_Array* str = (TypeV_Array*)typev_api_stack_pop_ptr(core);

    if(str->length == 4 && strncmp((char*)str->data, "true", 4) == 0){
        typev_api_return_u8(core, 1);
    }else if(str->length == 5 && strncmp((char*)str->data, "false", 5) == 0){
        typev_api_return_u8(core, 0);
    }else{
        typev_api_core_panic(core, RT_ERROR_CUSTOM, "Failed to convert string to bool\n");
    }
}

// use std lib for 1 and 2 bytes numbers

void string_toU8(TypeV_Core* core){
    TypeV_Array* str = (TypeV_Array*)typev_api_stack_pop_ptr(core);

    uint8_t value = strtol((char*)str->data, NULL, 10);

    typev_api_return_u8(core, (uint8_t)value);
}

void string_toI8(TypeV_Core* core) {
    TypeV_Array* str = (TypeV_Array*)typev_api_stack_pop_ptr(core);

    long value = strtol((char*)str->data, NULL, 10);

    // Check if the value is within the int8_t range
    if (value < INT8_MIN || value > INT8_MAX) {
        typev_api_core_panic(core, RT_ERROR_CUSTOM, "Value out of range for int8_t\n");
        return;
    }

    typev_api_return_i8(core, (int8_t)value);
}


void string_toU16(TypeV_Core* core){
    TypeV_Array* str = (TypeV_Array*)typev_api_stack_pop_ptr(core);

    uint16_t value = strtol((char*)str->data, NULL, 10);

    typev_api_return_u16(core, (uint16_t)value);
}

void string_toI16(TypeV_Core* core) {
    TypeV_Array* str = (TypeV_Array*)typev_api_stack_pop_ptr(core);

    long value = strtol((char*)str->data, NULL, 10);

    // Check if the value is within the int16_t range
    if (value < INT16_MIN || value > INT16_MAX) {
        typev_api_core_panic(core, RT_ERROR_CUSTOM, "Value out of range for int16_t\n");
        return;
    }

    typev_api_return_i16(core, (int16_t)value);
}

void string_toU32(TypeV_Core* core){
    TypeV_Array* str = (TypeV_Array*)typev_api_stack_pop_ptr(core);

    atoi_result res;
    char *endPtr = (char*)(str->data+str->length-1); // Declare a pointer to store the end position

    uint32_t value = atoi_u32_yy((char*)str->data, str->length, &endPtr, &res);

    if(res == atoi_result_fail){
        typev_api_core_panic(core, RT_ERROR_CUSTOM, "Failed to convert string to uint32_t\n");
        return;
    }
    if(res == atoi_result_overflow) {
        typev_api_core_panic(core, RT_ERROR_CUSTOM, "Failed to convert string to uint32_t, overflow\n");
        return;
    }

    typev_api_return_u32(core, (uint32_t)value);
}

void string_toI32(TypeV_Core* core){
    TypeV_Array* str = (TypeV_Array*)typev_api_stack_pop_ptr(core);

    atoi_result res;
    char* endPtr = (char*)str->data+str->length-1; // Declare a pointer to store the end position
    int32_t value = atoi_i32_yy((char*)str->data, str->length, &endPtr, &res);

    if(res == atoi_result_fail){
        typev_api_core_panic(core, RT_ERROR_CUSTOM, "Failed to convert string to int32_t\n");
        return;
    }
    if(res == atoi_result_overflow) {
        typev_api_core_panic(core, RT_ERROR_CUSTOM, "Failed to convert string to int32_t, overflow\n");
        return;
    }

    typev_api_return_i32(core, (int32_t)value);
}

void string_toU64(TypeV_Core* core){
    TypeV_Array* str = (TypeV_Array*)typev_api_stack_pop_ptr(core);

    atoi_result res;
    char *endPtr = (char*)(str->data+str->length-1);
    uint64_t value = atoi_u64_yy((char*)str->data, str->length, &endPtr, &res);

    if(res == atoi_result_fail){
        typev_api_core_panic(core, RT_ERROR_CUSTOM, "Failed to convert string to uint64_t\n");
        return;
    }
    if(res == atoi_result_overflow) {
        typev_api_core_panic(core, RT_ERROR_CUSTOM, "Failed to convert string to uint64_t, overflow\n");
        return;
    }

    typev_api_return_u64(core, (uint64_t)value);
}

void string_toI64(TypeV_Core* core){
    TypeV_Array* str = (TypeV_Array*)typev_api_stack_pop_ptr(core);

    atoi_result res;
    char *endPtr = (char*)(str->data+str->length-1);
    int64_t value = atoi_i64_yy((char*)str->data, str->length, &endPtr, &res);

    if(res == atoi_result_fail){
        typev_api_core_panic(core, RT_ERROR_CUSTOM, "Failed to convert string to int64_t\n");
        return;
    }
    if(res == atoi_result_overflow) {
        typev_api_core_panic(core, RT_ERROR_CUSTOM, "Failed to convert string to int64_t, overflow\n");
        return;
    }

    typev_api_return_i64(core, (int64_t)value);
}

/**
 * Date time API
 */

void _dt_now(TypeV_Core* core){
    bool valid = true;
    dt_t dt = dt_now(&valid);

    typev_api_return_u8(core, valid);
    typev_api_return_u16(core, dt.millisecond);
    typev_api_return_u8(core, dt.second);
    typev_api_return_u8(core, dt.minute);
    typev_api_return_u8(core, dt.hour);
    typev_api_return_u8(core, dt.day);
    typev_api_return_u8(core, dt.month);
    typev_api_return_u16(core, dt.year);
}

void _dt_parse(TypeV_Core* core){
    TypeV_Array* str = (TypeV_Array*)typev_api_stack_pop_ptr(core);
    bool valid = true;
    dt_t dt = dt_parse((char*)str->data, &valid);

    typev_api_return_u8(core, valid);
    typev_api_return_u16(core, dt.millisecond);
    typev_api_return_u8(core, dt.second);
    typev_api_return_u8(core, dt.minute);
    typev_api_return_u8(core, dt.hour);
    typev_api_return_u8(core, dt.day);
    typev_api_return_u8(core, dt.month);
    typev_api_return_u16(core, dt.year);
}

void _dt_toUnixTimestamp(TypeV_Core* core){
    uint16_t year = typev_api_stack_pop_u16(core);
    uint8_t month = typev_api_stack_pop_u8(core);
    uint8_t day = typev_api_stack_pop_u8(core);
    uint8_t hour = typev_api_stack_pop_u8(core);
    uint8_t minute = typev_api_stack_pop_u8(core);
    uint8_t second = typev_api_stack_pop_u8(core);
    uint16_t millisecond = typev_api_stack_pop_u16(core);

    dt_t dt = {year, month, day, hour, minute, second, millisecond};
    bool valid = true;
    int64_t timestamp = dt_toUnixTimestamp(&dt, &valid);

    typev_api_return_u8(core, valid);
    typev_api_return_i64(core, timestamp);
}

void _dt_fromUnixTimestamp(TypeV_Core* core){
    int64_t timestamp = typev_api_stack_pop_i64(core);
    bool valid = true;
    dt_t dt = dt_fromUnixTimestamp(timestamp, &valid);

    typev_api_return_u8(core, valid);
    typev_api_return_u16(core, dt.millisecond);
}

void _dt_getDayOfWeek(TypeV_Core* core){
    uint16_t year = typev_api_stack_pop_u16(core);
    uint8_t month = typev_api_stack_pop_u8(core);
    uint8_t day = typev_api_stack_pop_u8(core);

    dt_t dt = {year, month, day, 0, 0, 0, 0};
    bool valid = true;
    int dayOfWeek = dt_getDayOfWeek(&dt);
    if(dayOfWeek == -1){
        valid = false;
        dayOfWeek = 0;
    }

    typev_api_return_u8(core, valid);
    typev_api_return_u8(core, dayOfWeek);
}

void _dt_toLocalTime(TypeV_Core* core){
    uint16_t year = typev_api_stack_pop_u16(core);
    uint8_t month = typev_api_stack_pop_u8(core);
    uint8_t day = typev_api_stack_pop_u8(core);
    uint8_t hour = typev_api_stack_pop_u8(core);
    uint8_t minute = typev_api_stack_pop_u8(core);
    uint8_t second = typev_api_stack_pop_u8(core);
    uint16_t millisecond = typev_api_stack_pop_u16(core);

    dt_t dt = {year, month, day, hour, minute, second, millisecond};

    bool valid = true;
    dt_t local = dt_toLocalTime(&dt, &valid);

    typev_api_return_u8(core, valid);
    typev_api_return_u16(core, local.millisecond);
    typev_api_return_u8(core, local.second);
    typev_api_return_u8(core, local.minute);
    typev_api_return_u8(core, local.hour);
    typev_api_return_u8(core, local.day);
    typev_api_return_u8(core, local.month);
    typev_api_return_u16(core, local.year);
}



static TypeV_FFIFunc stdcore_lib[] = {
        double_to_str,
        float_to_str,
        bool_to_str,
        uint8_to_str,
        int8_to_str,
        uint16_to_str,
        int16_to_str,
        uint32_to_str,
        int32_to_str,
        uint64_to_str,
        int64_to_str,

        // string append
        string_append_f64,
        string_append_f32,
        string_append_u64,
        string_append_i64,
        string_append_u32,
        string_append_i32,
        string_append_u16,
        string_append_i16,
        string_append_u8,
        string_append_i8,
        string_append_bool,

        // string to number
        string_toF64,
        string_toF32,
        string_toBool,
        string_toU8,
        string_toI8,
        string_toU16,
        string_toI16,
        string_toU32,
        string_toI32,
        string_toU64,
        string_toI64,

        // datetime

        _dt_now,
        _dt_parse,
        _dt_toUnixTimestamp,
        _dt_fromUnixTimestamp,
        _dt_getDayOfWeek,
        _dt_toLocalTime,

        NULL
};

size_t typev_ffi_open(){
    return typev_api_register_lib(stdcore_lib);
}