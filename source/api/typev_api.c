//
// Created by praisethemoon on 02.12.23.
//

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "typev_api.h"
#include "../core.h"
#include "../stack/stack.h"


size_t typev_api_register_lib(const TypeV_FFIFunc methods[]) {
    // get the number of methods
    uint8_t methodCount = 0;
    while(methods[methodCount] != NULL) {
        methodCount++;
    }

    TypeV_FFI *ffi = malloc(sizeof(TypeV_FFI));
    ffi->functions = methods;
    ffi->functionCount = methodCount;

    return (size_t)ffi;
}

size_t typev_api_get_const_address(struct TypeV_Core* core, size_t vm_adr) {
    return (size_t)core->constPtr+ vm_adr;
}

uint64_t typev_api_stack_getSize(TypeV_Core* core) {
    return core->funcState->sp;
}

int8_t typev_api_stack_pop_i8(TypeV_Core* core) {
    int8_t value;
    stack_pop_8(core, &value);
    return value;
}

uint8_t typev_api_stack_pop_u8(TypeV_Core* core) {
    uint8_t value;
    stack_pop_8(core, &value);
    return value;
}

int16_t typev_api_stack_pop_i16(TypeV_Core* core) {
    int16_t value;
    stack_pop_16(core, &value);
    return value;
}

uint16_t typev_api_stack_pop_u16(TypeV_Core* core) {
    uint16_t value;
    stack_pop_16(core, &value);
    return value;

}

int32_t typev_api_stack_pop_i32(TypeV_Core* core) {
    int32_t value;
    stack_pop_32(core, &value);
    return value;
}

uint32_t typev_api_stack_pop_u32(TypeV_Core* core) {
    uint32_t value;
    stack_pop_32(core, &value);
    return value;
}

int64_t typev_api_stack_pop_i64(TypeV_Core* core) {
    int64_t value;
    stack_pop_64(core, &value);
    return value;
}

uint64_t typev_api_stack_pop_u64(TypeV_Core* core) {
    uint64_t value;
    stack_pop_64(core, &value);
    return value;
}

size_t typev_api_stack_pop_ptr(struct TypeV_Core* core){
    size_t value;
    stack_pop_ptr(core, &value);
    return value;
}

float typev_api_stack_pop_f32(TypeV_Core* core) {
    float value;
    stack_pop_32(core, (uint32_t*)&value);
    return value;
}

double typev_api_stack_pop_f64(TypeV_Core* core) {
    double value;
    stack_pop_64(core, (uint64_t*)&value);
    return value;
}

TypeV_Struct* typev_api_stack_pop_struct(TypeV_Core* core){
    TypeV_Struct* value;
    stack_pop_ptr(core, (size_t*)&value);
    return value;
}

TypeV_Class* typev_api_stack_pop_class(TypeV_Core* core) {
    TypeV_Class* value;
    stack_pop_ptr(core, (size_t*)&value);
    return value;
}

TypeV_Interface* typev_api_stack_pop_interface(TypeV_Core* core) {
    TypeV_Interface* value;
    stack_pop_ptr(core, (size_t*)&value);
    return value;
}


TypeV_Array* typev_api_stack_pop_array(TypeV_Core* core) {
    TypeV_Array* value;
    stack_pop_ptr(core, (size_t*)&value);
    return value;
}

void typev_api_return_i8(TypeV_Core* core, int8_t value) {
    // set the return value to the register R20
    //core->registers.regs[20].i8 = value;
}

void typev_api_return_u8(TypeV_Core* core, uint8_t value) {
    // set the return value to the register R20
    //core->registers.regs[20].u8 = value;
}
void typev_api_return_i16(TypeV_Core* core, int16_t value) {
    // set the return value to the register R20
    //core->registers.regs[20].i16 = value;
}

void typev_api_return_u16(TypeV_Core* core, uint16_t value) {
    // set the return value to the register R20
    // core->registers.regs[20].u16 = value;
}

void typev_api_return_i32(TypeV_Core* core, int32_t value) {
    // set the return value to the register R20
    // core->registers.regs[20].i32 = value;
}

void typev_api_return_u32(TypeV_Core* core, uint32_t value) {
    // set the return value to the register R20
    // core->registers.regs[20].u32 = value;
}

void typev_api_return_i64(TypeV_Core* core, int64_t value) {
    // core->registers.regs[20].i64 = value;
}

void typev_api_return_u64(TypeV_Core* core, uint64_t value) {
    // core->registers.regs[20].u64 = value;
}


void typev_api_return_ptr(TypeV_Core* core, size_t value) {
    // core->registers.regs[20].ptr = value;
}

void typev_api_return_f32(TypeV_Core* core, float value) {
    // core->registers.regs[20].f32 = value;
}

void typev_api_return_f64(TypeV_Core* core, double value) {
    // core->registers.regs[20].f64 = value;
}

void typev_api_return_struct(TypeV_Core* core, TypeV_Struct* value) {
    // core->registers.regs[20].ptr = (size_t)value;
}

void typev_api_return_class(TypeV_Core* core, TypeV_Class* value) {
    // core->registers.regs[20].ptr = (size_t)value;
}

void typev_api_return_interface(TypeV_Core* core, TypeV_Interface* value) {
    // core->registers.regs[20].ptr = (size_t)value;
}

void typev_api_return_array(TypeV_Core* core, TypeV_Array* value){
    // core->registers.regs[20].ptr = (size_t)value;
}

/**
* Structs
*/
TypeV_Struct *typev_api_struct_create(TypeV_Core *core, uint16_t fieldCount, size_t structSize) {
    // allocate memory for the struct
    TypeV_Struct *structPtr = calloc(1, sizeof(TypeV_Struct) + structSize);
    // set the field count
    structPtr->fieldOffsets = calloc(fieldCount, sizeof(uint16_t));

    // not used because it's already allocated alongside the struct, contiguous memory
    //structPtr->data = (uint8_t *) calloc(1, sizeof(structSize));

    return structPtr;
}

void typev_api_struct_set_offset(TypeV_Core *core, TypeV_Struct *structPtr, uint16_t fieldIndex, uint16_t offset) {
    structPtr->fieldOffsets[fieldIndex] = offset;
}

void typev_api_struct_set_field(TypeV_Core *core, TypeV_Struct *structPtr, uint16_t fieldIndex, void *value, size_t valueSize) {
    memcpy(structPtr->data + structPtr->fieldOffsets[fieldIndex], value, valueSize);
}

void typev_api_core_panic(TypeV_Core* core, uint32_t errorId, char* fmt, ...){
    core_panic(core, errorId, fmt);
};
