//
// Created by praisethemoon on 05.12.23.
//

#include <stdio.h>
#include "string.h"
#include "../core.h"
#include "../api/typev_api.h"
#include "../vendor/sds/sds.h"


/**
 * First internal functions
 */

typedef char (*char_map_func)(char);

sds mapSdsString(sds str, char_map_func mapFunc) {
    int len = sdslen(str);
    sds newStr = sdsnewlen(NULL, len);

    for (int i = 0; i < len; i++) {
        newStr[i] = mapFunc(str[i]);
    }

    return newStr;
}

char toUpperCase(char c) {
    if (c >= 'a' && c <= 'z') {
        return c - 32;
    }
    return c;
}

char toLowerCase(char c) {
    if (c >= 'A' && c <= 'Z') {
        return c + 32;
    }
    return c;
}

/**
 * TypeV functions
 */

void typev_std_string_from_const(TypeV_Core *core){
    size_t ptr = typev_api_stack_pop_ptr(core);
    char* p = (char*)typev_api_get_const_address(core, ptr);

    sds myString = sdsnew(p);

    typev_api_return_ptr(core, (size_t) myString);
}

void typev_std_string_empty(TypeV_Core *core) {
    sds myString = sdsempty();
    typev_api_return_ptr(core, (size_t) myString);
}

void typev_std_string_clear(TypeV_Core *core) {
    sds myString = (sds)typev_api_stack_pop_ptr(core);
    sdsclear(myString);
}

void typev_std_string_clone(TypeV_Core *core) {
    sds myString = (sds)typev_api_stack_pop_ptr(core);
    sds newString = sdsdup(myString);
    typev_api_return_ptr(core, (size_t) newString);
}

void typev_std_string_len(TypeV_Core *core) {
    sds myString = (sds)typev_api_stack_pop_ptr(core);
    size_t len = sdslen(myString);
    typev_api_return_u64(core, len);
}

void typev_std_string_makeRoomFor(TypeV_Core *core) {
    sds myString = (sds)typev_api_stack_pop_ptr(core);
    size_t size = typev_api_stack_pop_u64(core);
    myString = sdsMakeRoomFor(myString, size);
    typev_api_return_ptr(core, (size_t) myString);
}

void typev_std_string_freeSpace(TypeV_Core *core) {
    sds myString = (sds)typev_api_stack_pop_ptr(core);
    myString = sdsRemoveFreeSpace(myString);
    typev_api_return_ptr(core, (size_t) myString);
}

void typev_std_string_allocSize(TypeV_Core *core) {
    sds myString = (sds)typev_api_stack_pop_ptr(core);
    size_t size = sdsAllocSize(myString);
    typev_api_return_u64(core, size);
}

void typev_std_string_growZero(TypeV_Core *core) {
    size_t size = typev_api_stack_pop_u64(core);
    sds myString = (sds)typev_api_stack_pop_ptr(core);
    myString = sdsgrowzero(myString, size);
    typev_api_return_ptr(core, (size_t) myString);
}

void typev_std_string_catlen(TypeV_Core *core) {
    char* p = (char*)typev_api_get_const_address(core, typev_api_stack_pop_ptr(core));
    sds myString = (sds)typev_api_stack_pop_ptr(core);
    size_t len = typev_api_stack_pop_u64(core);
    myString = sdscatlen(myString, p, len);
    typev_api_return_ptr(core, (size_t) myString);
}

void typev_std_string_catconst(TypeV_Core *core) {
    sds myString = (sds)typev_api_stack_pop_ptr(core);
    char* p = (char*)typev_api_get_const_address(core, typev_api_stack_pop_ptr(core));
    myString = sdscat(myString, p);
    typev_api_return_ptr(core, (size_t) myString);
}

void typev_std_string_catstr(TypeV_Core *core) {
    sds str2 = (sds)typev_api_stack_pop_ptr(core);
    sds str1 = (sds)typev_api_stack_pop_ptr(core);
    sds myString = sdscatsds(str1, str2);
    typev_api_return_ptr(core, (size_t) myString);
}

void typev_std_string_fromI8(TypeV_Core *core) {
    // creates new string from i8
    int8_t value = typev_api_stack_pop_i8(core);
    sds myString = sdsempty();
    myString = sdscatprintf(myString, "%d", value);
    typev_api_return_ptr(core, (size_t) myString);
}

void typev_std_string_fromU8(TypeV_Core *core) {
    uint8_t value = typev_api_stack_pop_u8(core);
    sds myString = sdsempty();
    myString = sdscatprintf(myString, "%u", value);
    typev_api_return_ptr(core, (size_t) myString);
}

void typev_std_string_fromI16(TypeV_Core *core) {
    int16_t value = typev_api_stack_pop_i16(core);
    sds myString = sdsempty();
    myString = sdscatprintf(myString, "%d", value);
    typev_api_return_ptr(core, (size_t) myString);
}

void typev_std_string_fromU16(TypeV_Core *core) {
    uint16_t value = typev_api_stack_pop_u16(core);
    sds myString = sdsempty();
    myString = sdscatprintf(myString, "%u", value);
    typev_api_return_ptr(core, (size_t) myString);
}

void typev_std_string_fromI32(TypeV_Core *core) {
    int32_t value = typev_api_stack_pop_i32(core);
    sds myString = sdsempty();
    myString = sdscatprintf(myString, "%d", value);
    typev_api_return_ptr(core, (size_t) myString);
}

void typev_std_string_fromU32(TypeV_Core *core) {
    uint32_t value = typev_api_stack_pop_u32(core);
    sds myString = sdsempty();
    myString = sdscatprintf(myString, "%u", value);
    typev_api_return_ptr(core, (size_t) myString);
}

void typev_std_string_fromI64(TypeV_Core *core) {
    int64_t value = typev_api_stack_pop_i64(core);
    sds myString = sdsempty();
    myString = sdscatprintf(myString, "%lld", value);
    typev_api_return_ptr(core, (size_t) myString);
}

void typev_std_string_fromU64(TypeV_Core *core) {
    uint64_t value = typev_api_stack_pop_u64(core);
    sds myString = sdsempty();
    myString = sdscatprintf(myString, "%llu", value);
    typev_api_return_ptr(core, (size_t) myString);
}

void typev_std_string_fromF32(TypeV_Core *core) {
    float value = typev_api_stack_pop_f32(core);
    sds myString = sdsempty();
    myString = sdscatprintf(myString, "%f", value);
    typev_api_return_ptr(core, (size_t) myString);
}

void typev_std_string_fromF64(TypeV_Core *core) {
    double value = typev_api_stack_pop_f64(core);
    sds myString = sdsempty();
    myString = sdscatprintf(myString, "%lf", value);
    typev_api_return_ptr(core, (size_t) myString);
}

void typev_std_string_fromPtr(TypeV_Core *core) {
    sds myString = (sds)typev_api_stack_pop_ptr(core);
    size_t value = typev_api_stack_pop_ptr(core);
    myString = sdscatprintf(myString, "%zu", value);
    typev_api_return_ptr(core, (size_t) myString);
}

void typev_std_string_toI8(TypeV_Core *core) {
    sds myString = (sds)typev_api_stack_pop_ptr(core);
    uint8_t base = typev_api_stack_pop_u8(core);
    int8_t value = (int8_t)strtoll(myString, NULL, base);
    typev_api_return_i8(core, value);
}

void typev_std_string_toU8(TypeV_Core *core) {
    sds myString = (sds)typev_api_stack_pop_ptr(core);
    uint8_t base = typev_api_stack_pop_u8(core);
    uint8_t value = (uint8_t)strtoull(myString, NULL, base);
    typev_api_return_u8(core, value);
}

void typev_std_string_toI16(TypeV_Core *core) {
    sds myString = (sds)typev_api_stack_pop_ptr(core);
    uint8_t base = typev_api_stack_pop_u8(core);
    int16_t value = (int16_t)strtoll(myString, NULL, base);
    typev_api_return_i16(core, value);
}

void typev_std_string_toU16(TypeV_Core *core) {
    sds myString = (sds)typev_api_stack_pop_ptr(core);
    uint8_t base = typev_api_stack_pop_u8(core);
    uint16_t value = (uint16_t)strtoull(myString, NULL, base);
    typev_api_return_u16(core, value);
}

void typev_std_string_toI32(TypeV_Core *core) {
    sds myString = (sds)typev_api_stack_pop_ptr(core);
    uint8_t base = typev_api_stack_pop_u8(core);
    int32_t value = (int32_t)strtoll(myString, NULL, base);
    typev_api_return_i32(core, value);
}

void typev_std_string_toU32(TypeV_Core *core) {
    sds myString = (sds)typev_api_stack_pop_ptr(core);
    uint8_t base = typev_api_stack_pop_u8(core);
    uint32_t value = (uint32_t)strtoull(myString, NULL, base);
    typev_api_return_u32(core, value);
}

void typev_std_string_toI64(TypeV_Core *core) {
    sds myString = (sds)typev_api_stack_pop_ptr(core);
    uint8_t base = typev_api_stack_pop_u8(core);
    int64_t value = (int64_t)strtoll(myString, NULL, base);
    typev_api_return_i64(core, value);
}

void typev_std_string_toU64(TypeV_Core *core) {
    sds myString = (sds)typev_api_stack_pop_ptr(core);
    uint8_t base = typev_api_stack_pop_u8(core);
    uint64_t value = (uint64_t)strtoull(myString, NULL, base);
    typev_api_return_u64(core, value);
}

void typev_std_string_toF32(TypeV_Core *core) {
    sds myString = (sds)typev_api_stack_pop_ptr(core);
    float value = (float)strtod(myString, NULL);
    typev_api_return_f32(core, value);
}

void typev_std_string_toF64(TypeV_Core *core) {
    sds myString = (sds)typev_api_stack_pop_ptr(core);
    double value = strtod(myString, NULL);
    typev_api_return_f64(core, value);
}

void typev_std_string_toUpper(TypeV_Core *core) {
    sds myString = (sds)typev_api_stack_pop_ptr(core);
    sds newString = mapSdsString(myString, toUpperCase);
    typev_api_return_ptr(core, (size_t) newString);
}

void typev_std_string_toLower(TypeV_Core *core) {
    sds myString = (sds)typev_api_stack_pop_ptr(core);
    sds newString = mapSdsString(myString, toLowerCase);
    typev_api_return_ptr(core, (size_t) newString);
}

void typev_std_string_cmp(TypeV_Core *core) {
    sds myString = (sds)typev_api_stack_pop_ptr(core);
    char* p = (char*)typev_api_get_const_address(core, typev_api_stack_pop_ptr(core));
    uint8_t cmp = sdscmp(myString, p);
    typev_api_return_i32(core, cmp);
}

void typev_std_string_charAt(TypeV_Core *core) {
    sds myString = (sds)typev_api_stack_pop_ptr(core);
    uint64_t index = typev_api_stack_pop_u64(core);
    char c = myString[index];
    typev_api_return_u8(core, c);
}



static TypeV_FFIFunc string_lib[] = {
        &typev_std_string_from_const,
        &typev_std_string_empty,
        &typev_std_string_clear,
        &typev_std_string_clone,
        &typev_std_string_len,
        &typev_std_string_makeRoomFor,
        &typev_std_string_freeSpace,
        &typev_std_string_allocSize,
        &typev_std_string_growZero,
        &typev_std_string_catlen,
        &typev_std_string_catconst,
        &typev_std_string_catstr,
        &typev_std_string_fromI8,
        &typev_std_string_fromU8,
        &typev_std_string_fromI16,
        &typev_std_string_fromU16,
        &typev_std_string_fromI32,
        &typev_std_string_fromU32,
        &typev_std_string_fromI64,
        &typev_std_string_fromU64,
        &typev_std_string_fromF32,
        &typev_std_string_fromF64,
        &typev_std_string_fromPtr,
        &typev_std_string_toI8,
        &typev_std_string_toU8,
        &typev_std_string_toI16,
        &typev_std_string_toU16,
        &typev_std_string_toI32,
        &typev_std_string_toU32,
        &typev_std_string_toI64,
        &typev_std_string_toU64,
        &typev_std_string_toF32,
        &typev_std_string_toF64,
        &typev_std_string_toUpper,
        &typev_std_string_toLower,
        &typev_std_string_cmp,
        &typev_std_string_charAt,
        NULL
};

TypeV_FFIFunc* typev_std_string_get_lib(){
    return string_lib;
}