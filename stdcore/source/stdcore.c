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

#include "../../source/core.h"
#include "../../source/api/typev_api.h"

#define MAX_NUM_STR_SIZE 64

// Convert floating point types to string
void double_to_str(TypeV_Core *core) {
    char buffer[MAX_NUM_STR_SIZE] = {0};
    double value = typev_api_stack_pop_f64(core);

    int size = snprintf(buffer, MAX_NUM_STR_SIZE, "%f", value);
    TypeV_Array* str = (TypeV_Array*)core_array_alloc(core, size, 1);
    memcpy(str->data, buffer, size);

    typev_api_return_ptr(core, (size_t)str);
}

// Convert floating point types to string
void float_to_str(TypeV_Core *core) {
    char buffer[MAX_NUM_STR_SIZE] = {0};
    float value = typev_api_stack_pop_f32(core);

    int size = snprintf(buffer, MAX_NUM_STR_SIZE, "%f", value);
    TypeV_Array* str = (TypeV_Array*)core_array_alloc(core, size, 1);
    memcpy(str->data, buffer, size);

    typev_api_return_ptr(core, (size_t)str);
}

// Convert boolean type to string
void bool_to_str(TypeV_Core *core) {
    char buffer[MAX_NUM_STR_SIZE] = {0};
    uint8_t value = typev_api_stack_pop_u8(core);

    int size = snprintf(buffer, MAX_NUM_STR_SIZE, "%s", value?"true":"false");
    TypeV_Array* str = (TypeV_Array*)core_array_alloc(core, size, 1);
    memcpy(str->data, buffer, size);

    typev_api_return_ptr(core, (size_t)str);
}

void uint8_to_str(TypeV_Core *core) {
    char buffer[MAX_NUM_STR_SIZE] = {0};
    uint8_t value = typev_api_stack_pop_u8(core);

    int size = snprintf(buffer, MAX_NUM_STR_SIZE, "%" PRIu8, value);
    TypeV_Array* str = (TypeV_Array*)core_array_alloc(core, size, 1);
    memcpy(str->data, buffer, size);

    typev_api_return_ptr(core, (size_t)str);
}

void int8_to_str(TypeV_Core *core) {

    char buffer[MAX_NUM_STR_SIZE] = {0};
    int8_t value = typev_api_stack_pop_i8(core);

    int size = snprintf(buffer, MAX_NUM_STR_SIZE, "%" PRId8, value);
    TypeV_Array* str = (TypeV_Array*)core_array_alloc(core, size, 1);
    memcpy(str->data, buffer, size);

    typev_api_return_ptr(core, (size_t)str);
}


void uint16_to_str(TypeV_Core *core) {
    char buffer[MAX_NUM_STR_SIZE] = {0};
    uint16_t value = typev_api_stack_pop_u16(core);

    int size = snprintf(buffer, MAX_NUM_STR_SIZE, "%" PRIu16, value);
    TypeV_Array* str = (TypeV_Array*)core_array_alloc(core, size, 1);
    memcpy(str->data, buffer, size);

    typev_api_return_ptr(core, (size_t)str);
}

void int16_to_str(TypeV_Core *core) {

    char buffer[MAX_NUM_STR_SIZE] = {0};
    int16_t value = typev_api_stack_pop_i16(core);

    int size = snprintf(buffer, MAX_NUM_STR_SIZE, "%" PRId16, value);
    TypeV_Array* str = (TypeV_Array*)core_array_alloc(core, size, 1);
    memcpy(str->data, buffer, size);

    typev_api_return_ptr(core, (size_t)str);
}


void uint32_to_str(TypeV_Core *core) {
    char buffer[MAX_NUM_STR_SIZE] = {0};
    uint32_t value = typev_api_stack_pop_u32(core);

    int size = snprintf(buffer, MAX_NUM_STR_SIZE, "%" PRIu32, value);
    TypeV_Array* str = (TypeV_Array*)core_array_alloc(core, size, 1);
    memcpy(str->data, buffer, size);

    typev_api_return_ptr(core, (size_t)str);
}

void int32_to_str(TypeV_Core *core) {

    char buffer[MAX_NUM_STR_SIZE] = {0};
    int32_t value = typev_api_stack_pop_i32(core);

    int size = snprintf(buffer, MAX_NUM_STR_SIZE, "%" PRId32, value);
    TypeV_Array* str = (TypeV_Array*)core_array_alloc(core, size, 1);
    memcpy(str->data, buffer, size);

    typev_api_return_ptr(core, (size_t)str);
}


void uint64_to_str(TypeV_Core *core) {
    char buffer[MAX_NUM_STR_SIZE] = {0};
    uint64_t value = typev_api_stack_pop_u64(core);

    int size = snprintf(buffer, MAX_NUM_STR_SIZE, "%" PRIu64, value);
    TypeV_Array* str = (TypeV_Array*)core_array_alloc(core, size, 1);
    memcpy(str->data, buffer, size);

    typev_api_return_ptr(core, (size_t)str);
}

void int64_to_str(TypeV_Core *core) {

    char buffer[MAX_NUM_STR_SIZE] = {0};
    int64_t value = typev_api_stack_pop_i64(core);

    int size = snprintf(buffer, MAX_NUM_STR_SIZE, "%" PRId64, value);
    TypeV_Array* str = (TypeV_Array*)core_array_alloc(core, size, 1);
    memcpy(str->data, buffer, size);

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

    // convert the value to string
    int32_t gap = snprintf(str->data+pos, MAX_NUM_STR_SIZE, "%f", value);
    if(gap < 0){
        typev_api_core_panic(core, 1, "Failed to convert double to string, snprintf returned %d\n", gap);
    }

    typev_api_return_u32(core, gap);
}

#define MAX_FLOAT_STR_SIZE 20

void string_append_f32(TypeV_Core* core){
    TypeV_Array* str = (TypeV_Array*)typev_api_stack_pop_ptr(core);
    float value = typev_api_stack_pop_f32(core);
    uint32_t pos = typev_api_stack_pop_u32(core);

    // check if the array has enough capacity starting from pos
    if(str->length < pos + MAX_FLOAT_STR_SIZE ){
        // realloc the array
        core_array_extend(core, (size_t)str, pos+MAX_FLOAT_STR_SIZE);
    }

    // convert the value to string
    int32_t gap = snprintf(str->data+pos, MAX_FLOAT_STR_SIZE, "%f", value);
    if(gap < 0){
        core_panic(core, 1, "Failed to convert float to string, snprintf returned %d\n", gap);

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
    int32_t gap = snprintf(str->data+pos, MAX_UINT64_STR_SIZE, "%" PRIu64, value);
    if(gap < 0){
        core_panic(core, 1, "Failed to convert uint64_t to string, snprintf returned %d\n", gap);
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
    int32_t gap = snprintf(str->data+pos, MAX_INT64_STR_SIZE, "%" PRId64, value);
    if(gap < 0){
        core_panic(core, 1, "Failed to convert int64_t to string, snprintf returned %d\n", gap);
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
    int32_t gap = snprintf(str->data+pos, MAX_UINT32_STR_SIZE, "%" PRIu32, value);
    if(gap < 0){
        core_panic(core, 1, "Failed to convert uint32_t to string, snprintf returned %d\n", gap);
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

    // convert the value to string
    int32_t gap = snprintf(str->data+pos, MAX_INT32_STR_SIZE, "%" PRId32, value);
    if(gap < 0){
        core_panic(core, 1, "Failed to convert int32_t to string, snprintf returned %d\n", gap);
    }

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
    int32_t gap = snprintf(str->data+pos, MAX_UINT16_STR_SIZE, "%" PRIu16, value);
    if(gap < 0){
        core_panic(core, 1, "Failed to convert uint16_t to string, snprintf returned %d\n", gap);
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
    int32_t gap = snprintf(str->data+pos, MAX_INT16_STR_SIZE, "%" PRId16, value);
    if(gap < 0){
        core_panic(core, 1, "Failed to convert int16_t to string, snprintf returned %d\n", gap);
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
    int32_t gap = snprintf(str->data+pos, MAX_UINT8_STR_SIZE, "%" PRIu8, value);
    if(gap < 0){
        core_panic(core, 1, "Failed to convert uint8_t to string, snprintf returned %d\n");
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
    int32_t gap = snprintf(str->data+pos, MAX_INT8_STR_SIZE, "%" PRId8, value);
    if(gap < 0){
        core_panic(core, 1, "Failed to convert int8_t to string, snprintf returned %d\n", gap);
    }

    typev_api_return_u32(core, gap);
}

void string_append_bool(TypeV_Core* core){
    TypeV_Array* str = (TypeV_Array*)typev_api_stack_pop_ptr(core);
    uint8_t value = typev_api_stack_pop_u8(core);
    uint32_t pos = typev_api_stack_pop_u32(core);

    char* bool_str = value?"true":"false";
    int32_t gap = snprintf(str->data+pos, 5, "%s", bool_str);
    if(gap < 0){
        core_panic(core, 1, "Failed to convert bool to string, snprintf returned %d\n", gap);
    }

    typev_api_return_u32(core, gap);
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
        NULL
};

size_t typev_ffi_open(){
    return typev_api_register_lib(stdcore_lib);
}