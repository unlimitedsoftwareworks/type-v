/**
 * Type-V Virtual Machine
 * Author: praisethemoon
 * instructions.h: VM Instructions
 * VM instructions headers are defined here.
 */

#ifndef TYPE_V_INSTRUCTIONS_H
#define TYPE_V_INSTRUCTIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../core.h"
#include "./opcodes.h"

#include "../stack/stack.h"
#include "../gc/gc.h"
#include "instructions.h"
#include "../core.h"
#include "../utils/utils.h"
#include "../utils/log.h"
#include "../vendor/libtable/table.h"
#include "../engine.h"
#include "../errors/errors.h"

#define CORE_ASSERT(condition, message)



static inline void typev_memcpy_unaligned(void* dest, const void* src, size_t n) {
#if ARCH_SUPPORTS_UNALIGNED_ACCESS
    if (n == 8) {  // Common case optimization for 8 bytes
        *(uint64_t *)dest = *(const uint64_t *)src;
        return;
    }
#endif

    char* d = dest;
    const char* s = src;
    static void* dispatch_table[] = {&&DO_1, &&DO_2, &&DO_3, &&DO_4, &&DO_5, &&DO_6, &&DO_7, &&DO_8};
    goto *dispatch_table[n - 1];
    DO_8: d[7] = s[7];
    DO_7: d[6] = s[6];
    DO_6: d[5] = s[5];
    DO_5: d[4] = s[4];
    DO_4: d[3] = s[3];
    DO_3: d[2] = s[2];
    DO_2: d[1] = s[1];
    DO_1: d[0] = s[0];
}

// Function for copying 8 unaligned bytes as a uint64_t
static inline void typev_memcpy_unaligned_8(char* d, const uint8_t* s) {
#if ARCH_SUPPORTS_UNALIGNED_ACCESS
    *(uint64_t*)d = *(uint64_t*)s;
#else
    memcpy(d, s, 8);
#endif
}

// Read exactly 4 bytes from potentially unaligned memory and return as uint64_t (zero-extended)
static inline void typev_memcpy_unaligned_4(char*d, const uint8_t* s) {
#if ARCH_SUPPORTS_UNALIGNED_ACCESS
    *(uint32_t*)d = *(uint32_t*)s;
#else
    memcpy(d, s, 4);
#endif
}

// Read exactly 2 bytes from potentially unaligned memory and return as uint64_t (zero-extended)
static inline void typev_memcpy_unaligned_2(char* d, const uint8_t* s) {
#if ARCH_SUPPORTS_UNALIGNED_ACCESS
    *(uint16_t*)d = *(uint16_t*)s;
#else
    memcpy(d, s, 2);
#endif
}

// Read exactly 1 byte from potentially unaligned memory and return as uint64_t (zero-extended)
static inline void typev_memcpy_unaligned_1(char* d, const uint8_t* s) {
#if ARCH_SUPPORTS_UNALIGNED_ACCESS
    *(uint8_t*)d = *(uint8_t*)s;
#else
    d[0] = s[0];
#endif
}


static inline void typev_memcpy_aligned_8(void* dest, const void* src) {
    *(uint64_t *)dest = *(const uint64_t *)src;
}

static inline void typev_memcpy_aligned_4(void* dest, const void* src) {
    *(uint32_t *)dest = *(const uint32_t *)src;
}

static inline void typev_memcpy_aligned_2(void* dest, const void* src) {
    *(uint16_t *)dest = *(const uint16_t *)src;
}

static inline void typev_memcpy_aligned_1(void* dest, const void* src) {
    *(uint8_t *)dest = *(const uint8_t *)src;
}

static inline uint64_t typev_memcpy_u64(const unsigned char *s, size_t size) {
    uint64_t dest = 0;
    switch (size) {
        case 8: dest |= (uint64_t)s[7] << 56;  // Fall through
        case 7: dest |= (uint64_t)s[6] << 48;
        case 6: dest |= (uint64_t)s[5] << 40;
        case 5: dest |= (uint64_t)s[4] << 32;
        case 4: dest |= (uint64_t)s[3] << 24;
        case 3: dest |= (uint64_t)s[2] << 16;
        case 2: dest |= (uint64_t)s[1] << 8;
        case 1: dest |= (uint64_t)s[0];
    }
    return dest;
}



static inline void mv_reg_reg(TypeV_Core* core){
    const uint8_t target = core->codePtr[core->ip++];
    const uint8_t source = core->codePtr[core->ip++];

    core->ip++;
    //uint8_t byteSize = core->codePtr[core->ip++];
    // CORE_ASSERT(isValidByte(byteSize), "Invalid byte size");

    //typev_memcpy_unaligned(&core->regs[target], &core->regs[source], byteSize);
    core->regs[target].ptr = core->regs[source].ptr;
    CLEAR_REG_PTR(core->funcState, target);
}

static inline void mv_reg_reg_ptr(TypeV_Core* core){
    const uint8_t target = core->codePtr[core->ip++];
    const uint8_t source = core->codePtr[core->ip++];

    typev_memcpy_aligned_8((unsigned char *) &core->regs[target], (unsigned char *) &core->regs[source]);
    SET_REG_PTR(core->funcState, target);
}

static inline void mv_reg_null(TypeV_Core* core){
    const uint8_t target = core->codePtr[core->ip++];
    core->regs[target].ptr = 0;
    CLEAR_REG_PTR(core->funcState, target);
}

static inline void mv_reg_i(TypeV_Core* core){
    const uint8_t target = core->codePtr[core->ip++];
    uint8_t immediate_size = core->codePtr[core->ip++];
    //uint64_t immediate = 0;
    uint64_t immediate = typev_memcpy_u64(&core->codePtr[core->ip], immediate_size);
    // data could be unaligned
    //typev_memcpy_unaligned(&immediate, &core->codePtr[core->ip], immediate_size);
    core->ip += immediate_size;
    core->regs[target].ptr =  immediate;
    //typev_memcpy_unaligned(&core->regs[target], &immediate, immediate_size);
    //typev_memcpy_unaligned(&core->regs[target], &immediate, 8);
    CLEAR_REG_PTR(core->funcState, target);
}

static inline void mv_reg_i_ptr(TypeV_Core* core){
    const uint8_t target = core->codePtr[core->ip++];
    //uint8_t immediate_size = core->codePtr[core->ip++];
    uint64_t immediate = 0;
    //uint64_t immediate = typev_memcpy_unaligned_u64(&core->codePtr[core->ip], immediate_size);
    // data could be unaligned
    typev_memcpy_aligned_8((unsigned char *) &immediate, &core->codePtr[core->ip]);
    core->ip += 8;
    core->regs[target].ptr =  immediate;
    //typev_memcpy_unaligned(&core->regs[target], &immediate, immediate_size);
    //typev_memcpy_unaligned(&core->regs[target], &immediate, 8);
    SET_REG_PTR(core->funcState, target);
}

static inline void mv_reg_const(TypeV_Core* core){
    const uint8_t target = core->codePtr[core->ip++];
    const uint8_t offset_length = core->codePtr[core->ip++];
    size_t constant_offset = 0;
    // data could be unaligned
    typev_memcpy_unaligned((unsigned char *) &constant_offset, &core->codePtr[core->ip], offset_length);
    core->ip += offset_length;

    uint8_t byteSize = core->codePtr[core->ip++];

    CORE_ASSERT(isValidByte(byteSize), "Invalid byte size");

    typev_memcpy_unaligned(&core->regs[target], &core->constPtr[constant_offset], byteSize);
    CLEAR_REG_PTR(core->funcState, target);
}

static inline void mv_reg_const_ptr(TypeV_Core* core){
    const uint8_t target = core->codePtr[core->ip++];

    const uint8_t offset_length = core->codePtr[core->ip++];
    size_t constant_offset = 0;
    // data could be unaligned
    typev_memcpy_unaligned(&constant_offset, &core->codePtr[core->ip], offset_length);
    core->ip += offset_length;
    typev_memcpy_aligned_8(&core->regs[target], &core->constPtr[constant_offset]);
    SET_REG_PTR(core->funcState, target);
}

static inline void mv_global_reg(TypeV_Core* core){
    const uint8_t offset_length = core->codePtr[core->ip++];
    size_t offset = 0;
    typev_memcpy_unaligned(&offset, &core->codePtr[core->ip], offset_length);
    core->ip += offset_length;
    const uint8_t source = core->codePtr[core->ip++];
    uint8_t byteSize = core->codePtr[core->ip++];

    CORE_ASSERT(isValidByte(byteSize), "Invalid byte size");

    typev_memcpy_unaligned(&core->globalPtr[offset], &core->regs[source], byteSize);
}

static inline void mv_global_reg_ptr(TypeV_Core* core){
    const uint8_t offset_length = core->codePtr[core->ip++];
    size_t offset = 0;
    typev_memcpy_unaligned(&offset, &core->codePtr[core->ip], offset_length);
    core->ip += offset_length;
    const uint8_t source = core->codePtr[core->ip++];

    typev_memcpy_aligned_8(&core->globalPtr[offset], &core->regs[source]);
}

static inline void mv_reg_global(TypeV_Core* core){
    const uint8_t target = core->codePtr[core->ip++];
    const uint8_t offset_length = core->codePtr[core->ip++];
    size_t offset = 0;
    typev_memcpy_unaligned(&offset, &core->codePtr[core->ip], offset_length);
    core->ip += offset_length;
    uint8_t byteSize = core->codePtr[core->ip++];
    CORE_ASSERT(isValidByte(byteSize), "Invalid byte size");

    typev_memcpy_unaligned(&core->regs[target], &core->globalPtr[offset], byteSize);
}

static inline void mv_reg_global_ptr(TypeV_Core* core){
    const uint8_t target = core->codePtr[core->ip++];
    const uint8_t offset_length = core->codePtr[core->ip++];
    size_t offset = 0;
    typev_memcpy_unaligned(&offset, &core->codePtr[core->ip], offset_length);
    core->ip += offset_length;


    typev_memcpy_aligned_8(&core->regs[target], &core->globalPtr[offset]);
}

static inline void s_alloc(TypeV_Core* core){
    const uint8_t dest_reg = core->codePtr[core->ip++];
    ASSERT(dest_reg < MAX_REG, "Invalid register index");
    const uint8_t fields_count = core->codePtr[core->ip++];
    uint16_t struct_size;
    typev_memcpy_unaligned_2(&struct_size, &core->codePtr[core->ip]);
    core->ip += 2;

    // allocate memory for struct
    uintptr_t mem = core_struct_alloc(core, fields_count, struct_size);
    // move the pointer to R16
    core->regs[dest_reg].ptr = mem;
    SET_REG_PTR(core->funcState, dest_reg);
}

static inline void s_alloc_t(TypeV_Core* core){
    const uint8_t dest_reg = core->codePtr[core->ip++];
    const uint8_t template_offset = core->codePtr[core->ip++];
}

static inline void s_reg_field(TypeV_Core* core){
    const uint8_t src_reg = core->codePtr[core->ip++];
    uint8_t field_index = core->codePtr[core->ip++];

    uint32_t globalFieldIndex;
    typev_memcpy_unaligned_4(&globalFieldIndex, &core->codePtr[core->ip]);
    core->ip += 4;

    uint16_t offset;
    typev_memcpy_unaligned_2(&offset, &core->codePtr[core->ip]);
    core->ip += 2;

    uint8_t isPtr = core->codePtr[core->ip++];

    TypeV_Struct* struct_ptr = (TypeV_Struct*)core->regs[src_reg].ptr;
    struct_ptr->globalFields[field_index] = globalFieldIndex;
    struct_ptr->fieldOffsets[field_index] = offset;

    if (isPtr) {
        size_t byteIndex = field_index / 8;      // Determine which byte contains the bit
        uint8_t bitOffset = field_index % 8;     // Determine the bit position within the byte
        struct_ptr->pointerBitmask[byteIndex] |= (1 << bitOffset); // Set the bit to mark as a pointer
    }
}

static inline void s_loadf(TypeV_Core* core){
    char* code = (char*)core->codePtr+core->ip;
    const uint8_t target = *code;
    const uint8_t source = *(code+1);
    uint32_t field_index = typev_memcpy_u64(code+2, 4);
    //typev_memcpy_aligned_4(&field_index, code+2);


    uint8_t byteSize = *(code + 6);
    core->ip += 7;
    CORE_ASSERT(isValidByte(byteSize), "Invalid byte size");

    TypeV_Struct* struct_ptr = (TypeV_Struct*)core->regs[source].ptr;
    //TypeV_ObjectHeader *header = (TypeV_ObjectHeader *)core->regs[source].ptr;

    uint8_t index = object_find_global_index(core, struct_ptr->globalFields, struct_ptr->numFields, field_index);

    typev_memcpy_unaligned(&core->regs[target], ((char *) struct_ptr->data) + struct_ptr->fieldOffsets[index], byteSize);
    CLEAR_REG_PTR(core->funcState, target);
}

static inline void s_loadf_ptr(TypeV_Core* core){
    const uint8_t target = core->codePtr[core->ip++];

    const uint8_t source = core->codePtr[core->ip++];

    uint32_t field_index;
    typev_memcpy_unaligned_4(&field_index, &core->codePtr[core->ip]);
    core->ip += 4;

    TypeV_Struct* struct_ptr = (TypeV_Struct*)core->regs[source].ptr;
    uint8_t index = object_find_global_index(core, struct_ptr->globalFields, struct_ptr->numFields, field_index);
    typev_memcpy_aligned_8(&core->regs[target], ((char *) struct_ptr->data) + struct_ptr->fieldOffsets[index]);
    SET_REG_PTR(core->funcState, target);
}

static inline void s_storef_const(TypeV_Core* core){
    const uint8_t dest_reg = core->codePtr[core->ip++];
    uint32_t field_index = 0;
    typev_memcpy_unaligned_4(&field_index, &core->codePtr[core->ip]);
    core->ip += 4;

    uint32_t offset = 0;
    typev_memcpy_unaligned_4(&offset, &core->codePtr[core->ip]);
    core->ip += 4;
    uint8_t byteSize = core->codePtr[core->ip++];
    TypeV_Struct* struct_ptr = (TypeV_Struct*)core->regs[dest_reg].ptr;
    uint8_t index = object_find_global_index(core, struct_ptr->globalFields, struct_ptr->numFields, field_index);
    typev_memcpy_unaligned(((char *) struct_ptr->data) + struct_ptr->fieldOffsets[index], &core->constPtr[offset],
                           byteSize);
}

static inline void s_storef_const_ptr(TypeV_Core* core){
    const uint8_t dest_reg = core->codePtr[core->ip++];

    uint32_t field_index = 0;
    typev_memcpy_unaligned_4(&field_index, &core->codePtr[core->ip]);
    core->ip += 4;

    uint32_t offset;
    typev_memcpy_unaligned_4(&offset, &core->codePtr[core->ip]);
    core->ip += 4;
    TypeV_Struct* struct_ptr = (TypeV_Struct*)core->regs[dest_reg].ptr;
    uint8_t index = object_find_global_index(core, struct_ptr->globalFields, struct_ptr->numFields, field_index);
    typev_memcpy_unaligned_8(((char *) struct_ptr->data) + struct_ptr->fieldOffsets[index], &core->constPtr[offset]);

}

static inline void s_storef_reg(TypeV_Core* core){
    const uint8_t dest_reg = core->codePtr[core->ip++];

    uint32_t field_index;
    typev_memcpy_unaligned_4(&field_index, core->codePtr + core->ip);
    core->ip += 4;

    ASSERT(field_index < MAX_REG, "Invalid register index");
    const uint8_t source = core->codePtr[core->ip++];
    uint8_t byteSize = core->codePtr[core->ip++];
    CORE_ASSERT(isValidByte(byteSize), "Invalid byte size");

    TypeV_Struct *struct_ptr = (TypeV_Struct *) core->regs[dest_reg].ptr;
    uint8_t index = object_find_global_index(core, struct_ptr->globalFields, struct_ptr->numFields, field_index);
    typev_memcpy_unaligned(((char *) struct_ptr->data) + struct_ptr->fieldOffsets[index], &core->regs[source], byteSize);
}
static inline void s_storef_reg_ptr(TypeV_Core* core){
    const uint8_t dest_reg = core->codePtr[core->ip++];

    uint32_t field_index;
    typev_memcpy_unaligned_4(&field_index, &core->codePtr[core->ip]);
    core->ip += 4;

    ASSERT(field_index < MAX_REG, "Invalid register index");
    const uint8_t source = core->codePtr[core->ip++];

    TypeV_Struct *struct_ptr = (TypeV_Struct *) core->regs[dest_reg].ptr;
    uint8_t index = object_find_global_index(core, struct_ptr->globalFields, struct_ptr->numFields, field_index);
    typev_memcpy_aligned_8(((char *) struct_ptr->data) + struct_ptr->fieldOffsets[index], &core->regs[source]);
}

static inline void c_alloc(TypeV_Core* core){
    const uint8_t dest_reg = core->codePtr[core->ip++];
    ASSERT(dest_reg < MAX_REG, "Invalid register index");
    const uint8_t attrs_count = core->codePtr[core->ip++];

    const uint16_t methods_count;
    typev_memcpy_unaligned_2(&methods_count, &core->codePtr[core->ip]);
    core->ip += 2;

    uint16_t fields_size;
    typev_memcpy_unaligned_2(&fields_size, &core->codePtr[core->ip]);
    core->ip += 2;

    uint32_t classId = 0;
    typev_memcpy_unaligned_4(&classId, &core->codePtr[core->ip]);
    core->ip += 4;

    // allocate memory for class
    size_t mem = core_class_alloc(core, methods_count, attrs_count, fields_size, classId);
    // move the pointer to R17
    core->regs[dest_reg].ptr = mem;
    SET_REG_PTR(core->funcState, dest_reg);
}


static inline void c_alloc_t(TypeV_Core* core){
    const uint8_t dest_reg = core->codePtr[core->ip++];
    const uint8_t template_offset = core->codePtr[core->ip++];
}

static inline void c_reg_field(TypeV_Core* core){
    const uint8_t src_reg = core->codePtr[core->ip++];
    const uint8_t field_index = core->codePtr[core->ip++];

    uint16_t offset;
    typev_memcpy_unaligned_2(&offset, &core->codePtr[core->ip]);
    core->ip += 2;

    TypeV_Class* class_ptr = (TypeV_Class*)core->regs[src_reg].ptr;
    class_ptr->fieldOffsets[field_index] = offset;

    uint8_t isPtr = core->codePtr[core->ip++];


    if (isPtr) {
        size_t byteIndex = field_index / 8;      // Determine which byte contains the bit
        uint8_t bitOffset = field_index % 8;     // Determine the bit position within the byte
        class_ptr->pointerBitmask[byteIndex] |= (1 << bitOffset); // Set the bit to mark as a pointer
    }
}

static inline void c_storem(TypeV_Core* core){
    const uint8_t dest_reg = core->codePtr[core->ip++];
    ASSERT(dest_reg < MAX_REG, "Invalid register index");
    const uint8_t local_method_index = core->codePtr[core->ip++];
    uint32_t global_method_index;
    typev_memcpy_unaligned_4(&global_method_index, &core->codePtr[core->ip]);
    core->ip += 4;

    uint32_t method_address;
    typev_memcpy_unaligned_4(&method_address, &core->codePtr[core->ip]);
    core->ip += 4;

    TypeV_Class* c = (TypeV_Class*)core->regs[dest_reg].ptr;
    //LOG_INFO("Storing method %d at method_address %d in class %p", method_index, method_address, (void*)c);
    c->globalMethods[local_method_index] = global_method_index;
    c->methods[local_method_index] = method_address;
}

static inline void c_loadm(TypeV_Core* core){
    const uint8_t target = core->codePtr[core->ip++];
    const uint8_t class_reg = core->codePtr[core->ip++];
    uint32_t method_index;
    typev_memcpy_unaligned_4(&method_index, &core->codePtr[core->ip]);
    core->ip += 4;

    TypeV_Class* c = (TypeV_Class*)core->regs[class_reg].ptr;

    //uint8_t idx = class_find_global_index(c, method_index);
    uint8_t idx = object_find_global_index(core, c->globalMethods, c->numMethods, method_index);

    LOG_INFO("Loading method %d from class %p", method_index, (void*)c);

    size_t offset = c->methods[idx];
    core->regs[target].ptr = offset;
    CLEAR_REG_PTR(core->funcState, target);
}

static inline void c_storef_reg(TypeV_Core* core){
    const uint8_t class_reg = core->codePtr[core->ip++];
    const uint8_t fieldIndex = core->codePtr[core->ip++];
    const uint8_t source = core->codePtr[core->ip++];
    uint8_t byteSize = core->codePtr[core->ip++];
    CORE_ASSERT(isValidByte(byteSize), "Invalid byte size");

    TypeV_Class* c = (TypeV_Class*)core->regs[class_reg].ptr;
    size_t field_offset = c->fieldOffsets[fieldIndex];
    typev_memcpy_unaligned(c->data + field_offset, &core->regs[source], byteSize);
}

static inline void c_storef_reg_ptr(TypeV_Core* core){
    const uint8_t class_reg = core->codePtr[core->ip++];
    const uint8_t fieldIndex = core->codePtr[core->ip++];
    const uint8_t source = core->codePtr[core->ip++];

    TypeV_Class* c = (TypeV_Class*)core->regs[class_reg].ptr;
    size_t field_offset = c->fieldOffsets[fieldIndex];
    typev_memcpy_aligned_8(c->data + field_offset, &core->regs[source].ptr);
}

static inline void c_storef_const(TypeV_Core* core) {
    const uint8_t class_reg = core->codePtr[core->ip++];
    const uint8_t fieldIndex = core->codePtr[core->ip++];

    size_t offset = 0;
    typev_memcpy_aligned_4(&offset, &core->codePtr[core->ip]);
    core->ip += 8;
    uint8_t byteSize = core->codePtr[core->ip++];
    CORE_ASSERT(isValidByte(byteSize), "Invalid byte size");

    TypeV_Class *c = (TypeV_Class *) core->regs[class_reg].ptr;
    size_t field_offset = c->fieldOffsets[fieldIndex];

    typev_memcpy_unaligned(c->data + field_offset, &core->constPtr[offset], byteSize);
}

static inline void c_storef_const_ptr(TypeV_Core* core){
    const uint8_t class_reg = core->codePtr[core->ip++];
    const uint8_t fieldIndex = core->codePtr[core->ip++];
    uint32_t offset;
    typev_memcpy_unaligned_4(&offset, &core->codePtr[core->ip]);
    core->ip += 4;

    TypeV_Class* c = (TypeV_Class*)core->regs[class_reg].ptr;
    size_t field_offset = c->fieldOffsets[fieldIndex];
    typev_memcpy_unaligned_8(c->data + field_offset, &core->constPtr[offset]);
}

static inline void c_loadf(TypeV_Core* core){
    const uint8_t target = core->codePtr[core->ip++];
    const uint8_t class_reg = core->codePtr[core->ip++];
    const uint8_t fieldIndex = core->codePtr[core->ip++];
    uint8_t byteSize = core->codePtr[core->ip++];
    CORE_ASSERT(isValidByte(byteSize), "Invalid byte size");
    TypeV_Class* c = (TypeV_Class*)core->regs[class_reg].ptr;
    size_t field_offset = c->fieldOffsets[fieldIndex];
    typev_memcpy_unaligned(&core->regs[target], c->data + field_offset, byteSize);
    CLEAR_REG_PTR(core->funcState, target);
}

static inline void c_loadf_ptr(TypeV_Core* core){
    const uint8_t target = core->codePtr[core->ip++];
    const uint8_t class_reg = core->codePtr[core->ip++];
    const uint8_t fieldIndex = core->codePtr[core->ip++];

    TypeV_Class* c = (TypeV_Class*)core->regs[class_reg].ptr;
    size_t field_offset = c->fieldOffsets[fieldIndex];
    typev_memcpy_aligned_8(&core->regs[target], c->data + field_offset);
    CLEAR_REG_PTR(core->funcState, target);
}

static inline void i_is_c(TypeV_Core* core){
    const uint8_t target = core->codePtr[core->ip++];
    const uint8_t interface_reg = core->codePtr[core->ip++];

    uint64_t classId = 0;
    typev_memcpy_aligned_4(&classId, &core->codePtr[core->ip]);
    core->ip += 8;

    TypeV_Class* class_ = (TypeV_Class*)core->regs[interface_reg].ptr;

    core->regs[target].u8 = class_->uid == classId;
    CLEAR_REG_PTR(core->funcState, target);
}

static inline void i_has_m(TypeV_Core* core){
    uint32_t lookUpMethodId;
    typev_memcpy_unaligned_4(&lookUpMethodId, &core->codePtr[core->ip]);
    core->ip += 4;

    const uint8_t interface_reg = core->codePtr[core->ip++];

    uint32_t offset;
    typev_memcpy_unaligned_4(&offset, &core->codePtr[core->ip]);
    core->ip += 4;

    TypeV_Class * class_ = (TypeV_Class*)core->regs[interface_reg].ptr;

    uint8_t found = 0;

    // TODO: use bloom filters
    // or use other optimization, for its a naive search

    for(uint32_t i = 0; i < class_->numMethods; i++){
        if(class_->globalMethods[i] == lookUpMethodId){
            found = 1;
            break;
        }
    }

    if(!found){
        core->ip = offset;
    }
}

static inline void a_alloc(TypeV_Core* core){
    const uint8_t dest = core->codePtr[core->ip++];
    const uint8_t is_ptr = core->codePtr[core->ip++];
    uint64_t num_elements;
    typev_memcpy_unaligned_8(&num_elements, &core->codePtr[core->ip]);
    core->ip += 8;
    // next read the element size
    uint8_t element_size = core->codePtr[core->ip++];
    if(element_size > 8) {
        core_panic(core, RT_ERROR_ENTITY_TOO_LARGE, "Element size too large %d", element_size);
    }

    // allocate memory for struct
    size_t mem = core_array_alloc(core, is_ptr, num_elements, element_size);
    // move the pointer to R19
    core->regs[dest].ptr = mem;
    SET_REG_PTR(core->funcState, dest);
}

static inline void a_extend(TypeV_Core* core){
    const uint8_t target_array = core->codePtr[core->ip++];
    const uint8_t size_reg = core->codePtr[core->ip++];
    uint64_t num_elements = core->regs[size_reg].u64;

    size_t mem = core_array_extend(core, core->regs[target_array].ptr, num_elements);
    core->regs[target_array].ptr = mem;
}

static inline void a_len(TypeV_Core* core){
    const uint8_t target = core->codePtr[core->ip++];

    const uint8_t array_reg = core->codePtr[core->ip++];
    TypeV_Array* array = (TypeV_Array*)core->regs[array_reg].ptr;
    core->regs[target].u64 = array->length;
    CLEAR_REG_PTR(core->funcState, target);
}

static inline void a_slice(TypeV_Core* core){
    const uint8_t target = core->codePtr[core->ip++];
    const uint8_t source = core->codePtr[core->ip++];
    const uint8_t start = core->codePtr[core->ip++];
    const uint8_t end = core->codePtr[core->ip++];

    TypeV_Array* array = (TypeV_Array*)core->regs[source].ptr;
    uint64_t start_ = core->regs[start].u64;
    uint64_t end_ = core->regs[end].u64;

    size_t mem = core_array_slice(core, array, start_, end_);
    core->regs[target].ptr = mem;
    SET_REG_PTR(core->funcState, target);
}

static inline void a_insert_a(TypeV_Core* core){
    const uint8_t count_target = core->codePtr[core->ip++];
    const uint8_t arr_target = core->codePtr[core->ip++];
    const uint8_t arr_source = core->codePtr[core->ip++];
    const uint8_t index = core->codePtr[core->ip++];

    TypeV_Array* src_array = (TypeV_Array*)core->regs[arr_source].ptr;
    TypeV_Array* target_array = (TypeV_Array*)core->regs[arr_target].ptr;
    uint64_t idx = core->regs[index].u64;

    core->regs[count_target].u64 = core_array_insert(core, target_array, src_array, idx);
}

static inline void a_storef_reg(TypeV_Core* core){
    const uint8_t array_reg = core->codePtr[core->ip++];
    const uint8_t index = core->codePtr[core->ip++];
    const uint8_t source = core->codePtr[core->ip++];
    uint8_t byteSize = core->codePtr[core->ip++];
    CORE_ASSERT(isValidByte(byteSize), "Invalid byte size");

    TypeV_Array* array = (TypeV_Array*)core->regs[array_reg].ptr;
    ASSERT(core->regs[index].u64 < array->length, "Index out of bounds");
    typev_memcpy_unaligned(array->data + (core->regs[index].u64 * array->elementSize), &core->regs[source], byteSize);
}

static inline void a_storef_reg_ptr(TypeV_Core* core){
    const uint8_t array_reg = core->codePtr[core->ip++];
    const uint8_t index = core->codePtr[core->ip++];
    const uint8_t source = core->codePtr[core->ip++];

    TypeV_Array* array = (TypeV_Array*)core->regs[array_reg].ptr;

    if(core->regs[index].u64 >= array->length) {
        core_panic(core, RT_ERROR_OUT_OF_BOUNDS, "Index out of bounds %d >= %d", core->regs[index].u64, array->length);
    }
    typev_memcpy_aligned_8(array->data + (core->regs[index].u64 * array->elementSize), &core->regs[source]);
}

static inline void a_storef_const(TypeV_Core* core){
    const uint8_t array_reg = core->codePtr[core->ip++];
    uint8_t indexReg = core->codePtr[core->ip++];
    uint32_t offset;
    typev_memcpy_aligned_4(&offset, &core->codePtr[core->ip]);
    core->ip += 4;

    uint8_t byteSize = core->codePtr[core->ip++];
    CORE_ASSERT(isValidByte(byteSize), "Invalid byte size");

    TypeV_Array* array = (TypeV_Array*)core->regs[array_reg].ptr;
    ASSERT(core->regs[indexReg].u64 < array->length, "Index out of bounds");
    typev_memcpy_unaligned(array->data + (core->regs[indexReg].u64 * array->elementSize), &core->constPtr[offset],
                           byteSize);
}

static inline void a_storef_const_ptr(TypeV_Core* core){
    const uint8_t array_reg = core->codePtr[core->ip++];
    uint8_t indexReg = core->codePtr[core->ip++];
    uint32_t offset;
    typev_memcpy_aligned_4(&offset, &core->codePtr[core->ip]);
    core->ip += 4;
    TypeV_Array* array = (TypeV_Array*)core->regs[array_reg].ptr;
    ASSERT(core->regs[indexReg].u64 < array->length, "Index out of bounds");
    typev_memcpy_unaligned_8(array->data + (core->regs[indexReg].u64 * array->elementSize), &core->constPtr[offset]);
}

static inline void a_loadf(TypeV_Core* core) {
    const uint8_t target = core->codePtr[core->ip++];
    const uint8_t index = core->codePtr[core->ip++];
    const uint8_t array_reg = core->codePtr[core->ip++];

    TypeV_Array* array = (TypeV_Array*)core->regs[array_reg].ptr;

    uint64_t idx = core->regs[index].u64;

    if(idx >= array->length) {
        core_panic(core, RT_ERROR_OUT_OF_BOUNDS, "Index out of bounds %d >= %d", idx, array->length);
    }


    uint8_t byteSize = core->codePtr[core->ip++];
    CORE_ASSERT(isValidByte(byteSize), "Invalid byte size");

    typev_memcpy_unaligned(&core->regs[target], array->data + (idx * array->elementSize), byteSize);
    CLEAR_REG_PTR(core->funcState, target);
}

static inline void a_loadf_ptr(TypeV_Core* core) {
    const uint8_t target = core->codePtr[core->ip++];
    const uint8_t index = core->codePtr[core->ip++];
    const uint8_t array_reg = core->codePtr[core->ip++];

    TypeV_Array* array = (TypeV_Array*)core->regs[array_reg].ptr;

    uint64_t idx = core->regs[index].u64;
    if(idx >= array->length) {
        core_panic(core, 1, "Index out of bounds %d <= %d", idx, array->length);
        return;
    }

    typev_memcpy_aligned_8(&core->regs[target], array->data + (core->regs[index].u64 * array->elementSize));
    SET_REG_PTR(core->funcState, target);
}


// I: 915
static inline void push(TypeV_Core* core){
    const uint8_t source = core->codePtr[core->ip++];
    uint8_t bytesize = core->codePtr[core->ip++];
    if(bytesize == 0) bytesize = PTR_SIZE;


    ASSERT(bytesize <= 8, "Invalid byte size");

    switch(bytesize){
        case 1:
            stack_push_8(core->funcState, core->regs[source].u8);
            break;
        case 2:
            stack_push_16(core->funcState, core->regs[source].u16);
            break;
        case 4:
            stack_push_32(core->funcState, core->regs[source].u32);
            break;
        case 8:
            stack_push_64(core->funcState, core->regs[source].ptr);
            break;
        default:
            LOG_ERROR("Invalid byte size %d", bytesize);
            exit(-1);
    }
}

static inline void push_ptr(TypeV_Core* core){
    const uint8_t source = core->codePtr[core->ip++];
    stack_push_ptr(core->funcState, core->regs[source].ptr);
}

static inline void push_const(TypeV_Core* core){
    const uint8_t offset_length = core->codePtr[core->ip++];
    size_t offset = 0;
    typev_memcpy_unaligned(&offset, &core->codePtr[core->ip], offset_length);
    core->ip += offset_length;

    // get bytes
    uint8_t bytesize = core->codePtr[core->ip++];
    if(bytesize == 0) bytesize = PTR_SIZE;
    ASSERT(bytesize <= 8, "Invalid byte size");

    switch (bytesize) {
        case 0:
            stack_push_ptr(core->funcState, (size_t)&core->constPtr[offset]);
            break;
        case 1:
            stack_push_8(core->funcState, core->constPtr[offset]);
            break;
        case 2:
            stack_push_16(core->funcState, *((uint16_t*)&core->constPtr[offset]));
            break;
        case 4:
            stack_push_32(core->funcState, *((uint32_t*)&core->constPtr[offset]));
            break;
        case 8:
            stack_push_64(core->funcState, *((uint64_t*)&core->constPtr[offset]));
            break;
        default:
            LOG_ERROR("Invalid byte size %d", bytesize);
            exit(-1);
    }
}

static inline void pop(TypeV_Core* core){
    const uint8_t target = core->codePtr[core->ip++];
    uint8_t bytesize = core->codePtr[core->ip++];
    if(bytesize == 0) bytesize = PTR_SIZE;


    ASSERT(bytesize <= 8, "Invalid byte size");

    switch(bytesize){
        case 0:
            stack_pop_ptr(core->funcState, &core->regs[target].ptr);
            break;
        case 1:
            stack_pop_8(core->funcState, &core->regs[target].u8);
            break;
        case 2:
            stack_pop_16(core->funcState, &core->regs[target].u16);
            break;
        case 4:
            stack_pop_32(core->funcState, &core->regs[target].u32);
            break;
        case 8:
            stack_pop_64(core->funcState, &core->regs[target].u64);
            break;
        default:
            LOG_ERROR("Invalid byte size %d", bytesize);
            exit(-1);
    }
}

static inline void pop_ptr(TypeV_Core* core){
    const uint8_t target = core->codePtr[core->ip++];
    stack_pop_ptr(core->funcState, &core->regs[target].ptr);
}

static inline void fn_alloc(TypeV_Core* core){
    // creates a new function context, if needed, else reuse the old one
    TypeV_FuncState* newState;
    if(core->funcState->next == NULL) {
        newState = core_create_function_state(core->funcState);
    }
    else {
        newState = core->funcState->next;
        // clear the pointer status of the new state
        memset(newState->regsPtrBitmap, 0, sizeof(newState->regsPtrBitmap));
    }

    newState->prev = core->funcState;
    core->funcState->next = newState;
}

static inline void fn_set_reg(TypeV_Core* core){
    const uint8_t dest_reg = core->codePtr[core->ip++];
    const uint8_t source_reg = core->codePtr[core->ip++];

    core->ip++;
    //uint8_t byteSize = core->codePtr[core->ip++];
    CORE_ASSERT(isValidByte(byteSize), "Invalid byte size");

    //typev_memcpy_unaligned(&core->funcState->next->regs[dest_reg], &core->regs[source_reg], byteSize);
    core->funcState->next->regs[dest_reg] = core->regs[source_reg];
}

static inline void fn_set_reg_ptr(TypeV_Core* core){
    const uint8_t dest_reg = core->codePtr[core->ip++];
    const uint8_t source_reg = core->codePtr[core->ip++];

    core->funcState->next->regs[dest_reg] = core->regs[source_reg];
    SET_REG_PTR(core->funcState->next, dest_reg);
}

static inline void fn_call(TypeV_Core* core){
    // get the register
    const uint8_t target = core->codePtr[core->ip++];

    // read address
    const size_t adr = core->regs[target].ptr;

    // back up IP in funcState
    core->funcState->ip = core->ip;
    core->ip = adr;
    core->funcState = core->funcState->next;
    core->regs = core->funcState->regs;
}

static inline void fn_calli(TypeV_Core* core){
    //const uint8_t offset_length = core->codePtr[core->ip++];
    uint32_t offset;
    typev_memcpy_unaligned_4(&offset, &core->codePtr[core->ip]);
    //typev_memcpy_unaligned(&offset, &core->codePtr[core->ip],  offset_length);
    core->ip += 4;


    // jump to the address
    core->funcState->ip = core->ip;
    core->ip = offset;
    core->funcState = core->funcState->next;
    core->regs = core->funcState->regs;
}

static inline void fn_ret(TypeV_Core* core){
    core->ip = core->funcState->prev->ip;
    core->funcState = core->funcState->prev;
    core->regs = core->funcState->regs;
}

static inline void fn_get_ret_reg(TypeV_Core* core){
    const uint8_t dest_reg = core->codePtr[core->ip++];
    const uint8_t source_reg = core->codePtr[core->ip++];
    const uint8_t byteSize = core->codePtr[core->ip++];

    CORE_ASSERT(isValidByte(byteSize), "Invalid byte size");

    // we grab the register from the .next into the current one
    core->regs[dest_reg] = core->funcState->next->regs[source_reg];
    CLEAR_REG_PTR(core->funcState, dest_reg);
}

static inline void fn_get_ret_reg_ptr(TypeV_Core* core){
    const uint8_t dest_reg = core->codePtr[core->ip++];
    const uint8_t source_reg = core->codePtr[core->ip++];

    // we grab the register from the .next into the current one
    core->regs[dest_reg].ptr = core->funcState->next->regs[source_reg].ptr;
    SET_REG_PTR(core->funcState, dest_reg);
}

#define OP_CAST(d1, d2, type) \
static inline void cast_##d1##_##d2(TypeV_Core* core){ \
    uint8_t op1 = core->codePtr[core->ip++];\
    core->regs[op1].d2 = (type) core->regs[op1].d1;\
    CLEAR_REG_PTR(core->funcState, op1);\
}

OP_CAST(i8, u8, uint8_t)
OP_CAST(u8, i8, int8_t)
OP_CAST(i16, u16, uint16_t)
OP_CAST(u16, i16, int16_t)
OP_CAST(i32, u32, uint32_t)
OP_CAST(u32, i32, int32_t)
OP_CAST(i64, u64, uint64_t)
OP_CAST(u64, i64, int64_t)

OP_CAST(i32, f32, float)
OP_CAST(f32, i32, int32_t)
OP_CAST(i64, f64, double)
OP_CAST(f64, i64, int64_t)
#undef OP_CAST

static inline void upcast_i(TypeV_Core* core) {
    uint8_t reg = core->codePtr[core->ip++];
    uint8_t from = core->codePtr[core->ip++];
    uint8_t to = core->codePtr[core->ip++];

    ASSERT(reg < MAX_REG, "Invalid register index");
    ASSERT(from <= 8 && to <= 8 && from < to, "Invalid byte sizes for upcasting");

    // Extract the value from the register
    int64_t value = 0;
    typev_memcpy_unaligned(&value, &core->regs[reg], from);

    // Perform sign extension without branching
    if (from < 8) {
        // Create a sign mask to extend the sign bit if needed
        uint64_t signBit = 1ULL << (from * 8 - 1); // Extract the sign bit position
        uint64_t signExtMask = -(value & signBit); // Create the mask if sign bit is set
        value |= signExtMask;
    }

    // Store the result back in the register
    typev_memcpy_unaligned(&core->regs[reg], &value, to);
    CLEAR_REG_PTR(core->funcState, reg);
}


static inline void upcast_u(TypeV_Core* core) {
    uint8_t reg = core->codePtr[core->ip++];
    uint8_t from = core->codePtr[core->ip++];
    uint8_t to = core->codePtr[core->ip++];

    ASSERT(reg < MAX_REG, "Invalid register index");
    ASSERT(from <= 8 && to <= 8 && from < to, "Invalid byte sizes for upcasting");

    // Extract the value from the register
    uint64_t value = 0;
    typev_memcpy_unaligned(&value, &core->regs[reg], from);

    if(to == 2){
        core->regs[reg].u16 = (uint16_t) value;
    }
    else if(to == 4){
        core->regs[reg].u32 = (uint32_t) value;
    }
    else if(to == 8){
        core->regs[reg].u64 = (uint64_t) value;
    }
    else{
        LOG_ERROR("Invalid byte size %d", to);
        exit(-1);
    }

    // Store the result back in the register
    //typev_memcpy_unaligned(&core->regs[reg], &value, to);
    CLEAR_REG_PTR(core->funcState, reg);
}

static inline void upcast_f(TypeV_Core* core) {
    uint8_t reg = core->codePtr[core->ip++];
    uint8_t from = core->codePtr[core->ip++];
    uint8_t to = core->codePtr[core->ip++];

    ASSERT(reg < MAX_REG, "Invalid register index");
    ASSERT((from == 4 && to == 8), "Invalid byte sizes for floating-point upcasting");

    // Extract the float value from the register
    float floatValue = 0.0f;
    typev_memcpy_unaligned(&floatValue, &core->regs[reg], from);

    // Convert to double
    double doubleValue = (double)floatValue;

    // Store the result back in the register
    typev_memcpy_unaligned(&core->regs[reg], &doubleValue, to);
    CLEAR_REG_PTR(core->funcState, reg);
}


static inline void dcast_i(TypeV_Core* core) {
    uint8_t reg = core->codePtr[core->ip++];
    uint8_t from = core->codePtr[core->ip++];
    uint8_t to = core->codePtr[core->ip++];

    ASSERT(reg < MAX_REG, "Invalid register index");
    ASSERT(from <= 8 && to <= 8 && from > to, "Invalid byte sizes for downcasting");

    // We simply need to copy the lower 'to' bytes from the register
    int64_t value = 0;
    typev_memcpy_unaligned(&value, &core->regs[reg], to);

    // Store the result back in the register (overwriting only 'to' bytes)
    typev_memcpy_unaligned(&core->regs[reg], &value, to);
    CLEAR_REG_PTR(core->funcState, reg);
}

static inline void dcast_u(TypeV_Core* core) {
    uint8_t reg = core->codePtr[core->ip++];
    uint8_t from = core->codePtr[core->ip++];
    uint8_t to = core->codePtr[core->ip++];

    ASSERT(reg < MAX_REG, "Invalid register index");
    ASSERT(from <= 8 && to <= 8 && from > to, "Invalid byte sizes for downcasting");

    // Extract the value from the register
    uint64_t value = 0;
    typev_memcpy_unaligned(&value, &core->regs[reg], from);

    // Truncate the value to the smaller size
    uint64_t truncatedValue = 0;
    typev_memcpy_unaligned(&truncatedValue, &value, to);

    // Store the truncated value back in the register
    typev_memcpy_unaligned(&core->regs[reg], &truncatedValue, to);
    CLEAR_REG_PTR(core->funcState, reg);
}

static inline void dcast_f(TypeV_Core* core) {
    uint8_t reg = core->codePtr[core->ip++];
    uint8_t from = core->codePtr[core->ip++];
    uint8_t to = core->codePtr[core->ip++];

    ASSERT(reg < MAX_REG, "Invalid register index");
    ASSERT((from == 8 && to == 4), "Invalid byte sizes for floating-point downcasting");

    // Extract the double value from the register
    double doubleValue = 0.0;
    typev_memcpy_unaligned(&doubleValue, &core->regs[reg], from);

    // Convert to float
    float floatValue = (float)doubleValue;

    // Store the result back in the register
    typev_memcpy_unaligned(&core->regs[reg], &floatValue, to);
    CLEAR_REG_PTR(core->funcState, reg);
}



#define OP_BINARY(name, type, op)\
static inline void name##_##type(TypeV_Core* core){\
    uint8_t target = core->codePtr[core->ip++];\
    uint8_t op1 = core->codePtr[core->ip++];\
    uint8_t op2 = core->codePtr[core->ip++];\
    core->regs[target].type = core->regs[op1].type op core->regs[op2].type;\
    CLEAR_REG_PTR(core->funcState, target);\
}

OP_BINARY(add, i8, +)
OP_BINARY(add, u8, +)
OP_BINARY(add, i16, +)
OP_BINARY(add, u16, +)
OP_BINARY(add, i32, +)
//OP_BINARY(add, u32, +)

static inline void add_u32(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];
    uint8_t op2 = core->codePtr[core->ip++];
    //printf("adding %d + %d\n", core->regs[op1].u32, core->regs[op2].u32);
    core->regs[target].u32 = core->regs[op1].u32 + core->regs[op2].u32;
    CLEAR_REG_PTR(core->funcState, target);
}

OP_BINARY(add, i64, +)
//OP_BINARY(add, u64, +)

static inline void add_f32(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];
    uint8_t op2 = core->codePtr[core->ip++];
    //printf("adding %d + %d\n", core->regs[op1].u32, core->regs[op2].u32);
    core->regs[target].f32 = core->regs[op1].f32 + core->regs[op2].f32;
    CLEAR_REG_PTR(core->funcState, target);
}


OP_BINARY(add, f64, +)


static inline void add_u64(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];
    uint8_t op2 = core->codePtr[core->ip++];
    core->regs[target].u64 = core->regs[op1].u64 + core->regs[op2].u64;
    CLEAR_REG_PTR(core->funcState, target);
}

static inline void add_ptr_u8(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];
    uint8_t op2 = core->codePtr[core->ip++];
    core->regs[target].ptr = core->regs[op1].ptr + core->regs[op2].u8;
    CLEAR_REG_PTR(core->funcState, target);
}

static inline void add_ptr_u16(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];
    uint8_t op2 = core->codePtr[core->ip++];
    core->regs[target].ptr = core->regs[op1].ptr + core->regs[op2].u16;
    CLEAR_REG_PTR(core->funcState, target);
}

static inline void add_ptr_u32(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];
    uint8_t op2 = core->codePtr[core->ip++];
    core->regs[target].ptr = core->regs[op1].ptr + core->regs[op2].u32;
    CLEAR_REG_PTR(core->funcState, target);
}

static inline void add_ptr_u64(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];
    uint8_t op2 = core->codePtr[core->ip++];
    core->regs[target].ptr = core->regs[op1].ptr + core->regs[op2].u64;
    CLEAR_REG_PTR(core->funcState, target);
}

OP_BINARY(sub, i8, -)
OP_BINARY(sub, u8, -)
OP_BINARY(sub, i16, -)
OP_BINARY(sub, u16, -)
OP_BINARY(sub, i32, -)
OP_BINARY(sub, u32, -)
OP_BINARY(sub, i64, -)
OP_BINARY(sub, u64, -)
OP_BINARY(sub, f32, -)
OP_BINARY(sub, f64, -)

static inline void sub_ptr_u8(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];
    uint8_t op2 = core->codePtr[core->ip++];
    core->regs[target].ptr = core->regs[op1].ptr - core->regs[op2].u8;
    CLEAR_REG_PTR(core->funcState, target);
}

static inline void sub_ptr_u16(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];
    uint8_t op2 = core->codePtr[core->ip++];
    core->regs[target].ptr = core->regs[op1].ptr - core->regs[op2].u16;
    CLEAR_REG_PTR(core->funcState, target);
}

static inline void sub_ptr_u32(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];
    uint8_t op2 = core->codePtr[core->ip++];
    core->regs[target].ptr = core->regs[op1].ptr - core->regs[op2].u32;
    CLEAR_REG_PTR(core->funcState, target);
}

static inline void sub_ptr_u64(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];
    uint8_t op2 = core->codePtr[core->ip++];
    core->regs[target].ptr = core->regs[op1].ptr - core->regs[op2].u64;
    CLEAR_REG_PTR(core->funcState, target);
}

OP_BINARY(mul, i8, *)
OP_BINARY(mul, u8, *)
OP_BINARY(mul, i16, *)
OP_BINARY(mul, u16, *)
OP_BINARY(mul, i32, *)
OP_BINARY(mul, u32, *)
OP_BINARY(mul, i64, *)
OP_BINARY(mul, u64, *)
OP_BINARY(mul, f32, *)
OP_BINARY(mul, f64, *)

OP_BINARY(div, i8, /)
OP_BINARY(div, u8, /)
OP_BINARY(div, i16, /)
OP_BINARY(div, u16, /)
OP_BINARY(div, i32, /)
OP_BINARY(div, u32, /)
OP_BINARY(div, i64, /)
OP_BINARY(div, u64, /)
OP_BINARY(div, f32, /)
OP_BINARY(div, f64, /)

OP_BINARY(mod, i8, %)
OP_BINARY(mod, u8, %)
OP_BINARY(mod, i16, %)
OP_BINARY(mod, u16, %)
OP_BINARY(mod, i32, %)
OP_BINARY(mod, u32, %)
OP_BINARY(mod, i64, %)
OP_BINARY(mod, u64, %)

OP_BINARY(lshift, i8, <<)
OP_BINARY(lshift, u8, <<)
OP_BINARY(lshift, i16, <<)
OP_BINARY(lshift, u16, <<)
OP_BINARY(lshift, i32, <<)
OP_BINARY(lshift, u32, <<)
OP_BINARY(lshift, i64, <<)
OP_BINARY(lshift, u64, <<)

OP_BINARY(rshift, i8, >>)
OP_BINARY(rshift, u8, >>)
OP_BINARY(rshift, i16, >>)
OP_BINARY(rshift, u16, >>)
OP_BINARY(rshift, i32, >>)
OP_BINARY(rshift, u32, >>)
OP_BINARY(rshift, i64, >>)
OP_BINARY(rshift, u64, >>)
#undef OP_BINARY


static inline void band_8(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];
    uint8_t op2 = core->codePtr[core->ip++];

    core->regs[target].u8 = core->regs[op1].u8 & core->regs[op2].u8;
    CLEAR_REG_PTR(core->funcState, target);
}

static inline void band_16(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];
    uint8_t op2 = core->codePtr[core->ip++];

    core->regs[target].u16 = core->regs[op1].u16 & core->regs[op2].u16;
    CLEAR_REG_PTR(core->funcState, target);
}

static inline void band_32(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];
    uint8_t op2 = core->codePtr[core->ip++];

    core->regs[target].u32 = core->regs[op1].u32 & core->regs[op2].u32;
    CLEAR_REG_PTR(core->funcState, target);
}

static inline void band_64(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];
    uint8_t op2 = core->codePtr[core->ip++];

    core->regs[target].u64 = core->regs[op1].u64 & core->regs[op2].u64;
    CLEAR_REG_PTR(core->funcState, target);
}

static inline void bor_8(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];
    uint8_t op2 = core->codePtr[core->ip++];

    core->regs[target].u8 = core->regs[op1].u8 | core->regs[op2].u8;
    CLEAR_REG_PTR(core->funcState, target);
}

static inline void bor_16(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];
    uint8_t op2 = core->codePtr[core->ip++];

    core->regs[target].u16 = core->regs[op1].u16 | core->regs[op2].u16;
    CLEAR_REG_PTR(core->funcState, target);
}

static inline void bor_32(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];
    uint8_t op2 = core->codePtr[core->ip++];

    core->regs[target].u32 = core->regs[op1].u32 | core->regs[op2].u32;
    CLEAR_REG_PTR(core->funcState, target);
}

static inline void bor_64(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];
    uint8_t op2 = core->codePtr[core->ip++];

    core->regs[target].u64 = core->regs[op1].u64 | core->regs[op2].u64;
    CLEAR_REG_PTR(core->funcState, target);
}

static inline void bxor_8(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];

    core->regs[target].u8 = core->regs[op1].u8 ^ core->regs[target].u8;
    CLEAR_REG_PTR(core->funcState, target);
}

static inline void bxor_16(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];

    core->regs[target].u16 = core->regs[op1].u16 ^ core->regs[target].u16;
    CLEAR_REG_PTR(core->funcState, target);
}

static inline void bxor_32(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];

    core->regs[target].u32 = core->regs[op1].u32 ^ core->regs[target].u32;
    CLEAR_REG_PTR(core->funcState, target);
}

static inline void bxor_64(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];

    core->regs[target].u64 = core->regs[op1].u64 ^ core->regs[target].u64;
    CLEAR_REG_PTR(core->funcState, target);
}

static inline void bnot_8(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];

    core->regs[target].u8 = ~core->regs[op1].u8;
    CLEAR_REG_PTR(core->funcState, target);
}

static inline void bnot_16(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];

    core->regs[target].u16 = ~core->regs[op1].u16;
    CLEAR_REG_PTR(core->funcState, target);
}

static inline void bnot_32(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];

    core->regs[target].u32 = ~core->regs[op1].u32;
    CLEAR_REG_PTR(core->funcState, target);
}

static inline void bnot_64(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];

    core->regs[target].u64 = ~core->regs[op1].u64;
    CLEAR_REG_PTR(core->funcState, target);
}

static inline void and(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];
    uint8_t op2 = core->codePtr[core->ip++];

    core->regs[target].u8 = core->regs[op1].u8 && core->regs[op2].u8;
    CLEAR_REG_PTR(core->funcState, target);
}

static inline void or(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];
    uint8_t op2 = core->codePtr[core->ip++];

    core->regs[target].u8 = core->regs[op1].u8 || core->regs[op2].u8;
    CLEAR_REG_PTR(core->funcState, target);
}

static inline void not(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];

    core->regs[target].u8 = !core->regs[op1].u8;
    CLEAR_REG_PTR(core->funcState, target);
}

static inline void jmp(TypeV_Core* core){
    core->ip++;
    //const uint8_t offset_length = core->codePtr[core->ip++];
    uint32_t offset;
    typev_memcpy_unaligned_4(&offset, &core->codePtr[core->ip]);
    core->ip = offset;
}


#define OP_CMP(name, type)\
static inline void j_cmp_##name(TypeV_Core* core) {\
    uint8_t op1 = core->codePtr[core->ip++];\
    uint8_t op2 = core->codePtr[core->ip++];\
    uint8_t cmpType = core->codePtr[core->ip++];\
    uint32_t offset;\
    typev_memcpy_unaligned_4(&offset, &core->codePtr[core->ip]);\
    core->ip += 4;\
    type v1 = core->regs[op1].name;\
    type v2 = core->regs[op2].name;\
    switch(cmpType) {\
        case 0:\
            if(v1 == v2) core->ip = offset;\
            break;\
        case 1:\
            if(v1 != v2) core->ip = offset;\
            break;\
        case 2:\
            if(v1 > v2) core->ip = offset;\
            break;\
        case 3:\
            if(v1 >= v2) core->ip = offset;\
            break;\
        case 4:\
            if(v1 < v2) core->ip = offset;\
            break;\
        case 5:\
            if(v1 <= v2) core->ip = offset;\
            break;\
        default:\
            core_panic(core, RT_ERROR_INVALID_COMPARISON_OPERATOR, "Invalid comparison type");\
            exit(-1);\
    }\
}\


//OP_CMP(u8, uint8_t)

static inline void j_cmp_u8(TypeV_Core* core) {
    uint8_t op1 = core->codePtr[core->ip++];
    uint8_t op2 = core->codePtr[core->ip++];
    uint8_t cmpType = core->codePtr[core->ip++];
    uint32_t offset;
    typev_memcpy_unaligned_4(&offset, &core->codePtr[core->ip]);
    core->ip += 4;
    uint8_t v1 = core->regs[op1].u8;
    uint8_t v2 = core->regs[op2].u8;
    switch(cmpType) {
        case 0:
            if(v1 == v2) core->ip = offset;
            break;
        case 1:
            if(v1 != v2) core->ip = offset;
            break;
        case 2:
            if(v1 > v2) core->ip = offset;
            break;
        case 3:
            if(v1 >= v2) core->ip = offset;
            break;
        case 4:
            if(v1 < v2) core->ip = offset;
            break;
        case 5:
            if(v1 <= v2) core->ip = offset;
            break;
        default:
            core_panic(core, RT_ERROR_INVALID_COMPARISON_OPERATOR, "Invalid comparison type");\
            exit(-1);
    }
}

OP_CMP(i8, int8_t)
OP_CMP(u16, uint16_t)
OP_CMP(i16, int16_t)
//OP_CMP(u32, uint32_t)

static inline void j_cmp_u32(TypeV_Core* core) {
    uint8_t op1 = core->codePtr[core->ip++];
    uint8_t op2 = core->codePtr[core->ip++];
    uint8_t cmpType = core->codePtr[core->ip++];
    uint32_t offset;
    typev_memcpy_unaligned_4(&offset, &core->codePtr[core->ip]);
    core->ip += 4;
    uint32_t v1 = core->regs[op1].u32;
    uint32_t v2 = core->regs[op2].u32;
    switch(cmpType) {
        case 0:
            if(v1 == v2) core->ip = offset;
            break;
        case 1:
            if(v1 != v2) core->ip = offset;
            break;
        case 2:
            if(v1 > v2) core->ip = offset;
            break;
        case 3:
            if(v1 >= v2) core->ip = offset;
            break;
        case 4:
            if(v1 < v2) core->ip = offset;
            break;
        case 5:
            if(v1 <= v2) core->ip = offset;
            break;
        default:
            core_panic(core, RT_ERROR_INVALID_COMPARISON_OPERATOR, "Invalid comparison type");\
            exit(-1);
    }
}

OP_CMP(i32, int32_t)

static inline void j_cmp_u64(TypeV_Core* core) {
    uint8_t op1 = core->codePtr[core->ip++];
    uint8_t op2 = core->codePtr[core->ip++];
    uint8_t cmpType = core->codePtr[core->ip++];
    uint32_t offset;
    typev_memcpy_unaligned_4(&offset, &core->codePtr[core->ip]);
    core->ip += 4;
    uint64_t v1 = core->regs[op1].u32;
    uint64_t v2 = core->regs[op2].u32;
    switch(cmpType) {
        case 0:
            if(v1 == v2) core->ip = offset;
            break;
        case 1:
            if(v1 != v2) core->ip = offset;
            break;
        case 2:
            if(v1 > v2) core->ip = offset;
            break;
        case 3:
            if(v1 >= v2) core->ip = offset;
            break;
        case 4:
            if(v1 < v2) core->ip = offset;
            break;
        case 5:
            if(v1 <= v2) core->ip = offset;
            break;
        default:
            core_panic(core, RT_ERROR_INVALID_COMPARISON_OPERATOR, "Invalid comparison type");\
            exit(-1);
    }
}

OP_CMP(i64, int64_t)
OP_CMP(f32, float)
OP_CMP(f64, double)
//OP_CMP(ptr, uintptr_t)

static inline void j_cmp_ptr(TypeV_Core* core) {
    uint8_t op1 = core->codePtr[core->ip++];
    uint8_t op2 = core->codePtr[core->ip++];
    uint8_t cmpType = core->codePtr[core->ip++];
    uint32_t offset;
    typev_memcpy_unaligned_4(&offset, &core->codePtr[core->ip]);
    core->ip += 4;
    uintptr_t v1 = core->regs[op1].ptr;
    uintptr_t v2 = core->regs[op2].ptr;
    switch(cmpType) {
        case 0:
            if(v1 == v2) core->ip = offset;
            break;
        case 1:
            if(v1 != v2) core->ip = offset;
            break;
        case 2:
            if(v1 > v2) core->ip = offset;
            break;
        case 3:
            if(v1 >= v2) core->ip = offset;
            break;
        case 4:
            if(v1 < v2) core->ip = offset;
            break;
        case 5:
            if(v1 <= v2) core->ip = offset;
            break;
        default:
            core_panic(core, RT_ERROR_INVALID_COMPARISON_OPERATOR, "Invalid comparison type");\
            exit(-1);
    }
}

#undef OP_CMP


static inline void j_cmp_bool(TypeV_Core* core) {
    uint8_t op1 = core->codePtr[core->ip++];
    uint8_t op2 = core->codePtr[core->ip++];
    uint8_t cmpType = core->codePtr[core->ip++];
    uint32_t offset;
    typev_memcpy_unaligned_4(&offset, &core->codePtr[core->ip]);
    core->ip += 4;
    // Fetch the register values once, store them in registers
    uint8_t v1 = core->regs[op1].u8;
    uint8_t v2 = core->regs[op2].u8;

    // Compute the comparison result using bitwise XOR for == and !=
    uint8_t result = !v1 == !v2; // result will be 0 if v1 == v2, non-zero otherwise


    // Now result is 0 if the condition is true (either == or !=)

    // If the result is zero, jump to the offset by setting the instruction pointer
    //core->ip = core->ip + (result ? 0 : (offset - core->ip));
    if(result && (cmpType == 0)){
        core->ip = offset;
    }
    else if(!result && (cmpType == 1)){
        core->ip = offset;
    }else {
        core_panic(core, RT_ERROR_INVALID_COMPARISON_OPERATOR, "Invalid comparison type");
    }
}


static inline void reg_ffi(TypeV_Core* core){
    uint8_t offsetSize = core->codePtr[core->ip++];
    size_t offset = 0;
    memcpy(&offset, &core->codePtr[core->ip], offsetSize);
    core->ip += offsetSize;

    uint16_t id = 0;
    memcpy(&id, &core->codePtr[core->ip], 2);
    core->ip += 2;

    const char* namePtr = (const char*)&core->constPtr[offset];
    char* name = strdup(namePtr);
    engine_ffi_register(core->engineRef, name, id);
}

static inline void open_ffi(TypeV_Core* core){
    uint16_t ffi_id;
    typev_memcpy_unaligned_2(&ffi_id, &core->codePtr[core->ip]);
    core->ip += 2;

    engine_ffi_open(core->engineRef, ffi_id);
}

static inline void ld_ffi(TypeV_Core* core){
    uint8_t dest = core->codePtr[core->ip++];
    ASSERT(dest < MAX_REG, "Invalid register index");

    uint16_t id;
    typev_memcpy_unaligned_2(&id, &core->codePtr[core->ip]);

    core->ip += 2;

    uint8_t methodId = core->codePtr[core->ip++];

    core->regs[dest].ptr = (size_t)engine_ffi_get(core->engineRef, id, methodId);
}

static inline void call_ffi(TypeV_Core* core){
    uint8_t reg = core->codePtr[core->ip++];

    TypeV_FFIFunc ffi_fn = (TypeV_FFIFunc)(core->regs[reg].ptr);
    ffi_fn(core);
}

static inline void close_ffi(TypeV_Core* core){
    uint8_t reg = core->codePtr[core->ip++];
    core_ffi_close(core, core->regs[reg].ptr);
}

static inline void debug_reg(TypeV_Core* core){
    // read register index
    uint8_t i = core->codePtr[core->ip++];
    ASSERT(i < MAX_REG, "Invalid register index");

    struct table t;
    table_init(&t,
               "Core", "%d",
               "Reg", "R%d",
               "i8", "%d",
               "u8", "%u",
               "i16", "%d",
               "u16", "%u",
               "i32", "%d",
               "u32", "%u",
               "i64", "%lld",
               "u64", "%llu",
               "f32", "%f",
               "f64", "%lf",
               "ptr", "%p",
               "hex", "%llx", NULL);

    table_add(&t, core->id, i,
              core->regs[i].i8,
              core->regs[i].u8,
              core->regs[i].i16,
              core->regs[i].u16,
              core->regs[i].i32,
              core->regs[i].u32,
              core->regs[i].i64,
              core->regs[i].u64,
              core->regs[i].f32,
              core->regs[i].f64,
              (void*)core->regs[i].ptr,
              core->regs[i].u64);
    table_print(&t, 2000, stdout);
    table_free(&t);
}

static inline void halt(TypeV_Core* core) {
    uint8_t code_reg = core->codePtr[core->ip++];
    ASSERT(code_reg < MAX_REG, "Invalid register index");

    uint32_t code = core->regs[code_reg].u32;

    //core_gc_sweep_all(core);

    exit(code);
}

static inline void load_std(TypeV_Core* core){
}

static inline void vm_health(TypeV_Core* core){
    uint8_t dest = core->codePtr[core->ip++];
    ASSERT(dest < MAX_REG, "Invalid register index");

    core->regs[dest].u8 = core->engineRef->health;
}


static inline void closure_alloc(TypeV_Core* core) {
    uint8_t dest = core->codePtr[core->ip++];
    uint8_t offsetToArgs = core->codePtr[core->ip++];
    uint8_t envSize = core->codePtr[core->ip++];
    uint32_t fnAddressReg = 0;
    typev_memcpy_unaligned_4(&fnAddressReg, &core->codePtr[core->ip]);
    core->ip += 4;

    core->regs[dest].ptr = (uintptr_t) core_closure_alloc(core, fnAddressReg, offsetToArgs, envSize);
    SET_REG_PTR(core->funcState, dest);
}

static inline void closure_push_env(TypeV_Core* core) {
    uint8_t closureReg = core->codePtr[core->ip++];
    uint8_t regId = core->codePtr[core->ip++];
    uint8_t byteSize = core->codePtr[core->ip++];
    TypeV_Closure* cl = (TypeV_Closure*) core->regs[closureReg].ptr;
    // = core->regs[regId].ptr;
    typev_memcpy_unaligned(&cl->upvalues[cl->envCounter++], &core->regs[regId], byteSize);
}

static inline void closure_push_env_ptr(TypeV_Core* core) {
    uint8_t closureReg = core->codePtr[core->ip++];
    uint8_t regId = core->codePtr[core->ip++];
    TypeV_Closure* cl = (TypeV_Closure*) core->regs[closureReg].ptr;
    uintptr_t ptr = core->regs[regId].ptr;
    cl->upvalues[cl->envCounter].ptr = ptr;
    cl->ptrFields[cl->envCounter / 8] |= (1 << (cl->envCounter % 8));

    cl->envCounter++;
}

static inline void closure_call(TypeV_Core* core) {
    uint8_t closureReg = core->codePtr[core->ip++];
    TypeV_Closure* cl = (TypeV_Closure*) core->regs[closureReg].ptr;
    // sets the enviroment of the core->funcState to the closure's enviroment
    const size_t adr = cl->fnAddress;

    // back up IP in funcState
    core->funcState->ip = core->ip;
    core->ip = adr;
    core->funcState = core->funcState->next;
    core->regs = core->funcState->regs;

    uint8_t offset = cl->offset;
    // copy the closure's environment to the function's environment
    for (uint8_t i = 0; i < cl->envSize; i++) {
        core->funcState->regs[i+offset] = cl->upvalues[i];
        if(IS_CLOSURE_UPVALUE_POINTER(cl->ptrFields, i)) {
            SET_REG_PTR(core->funcState, i+offset);
        }
    }
}

static inline void closure_backup(TypeV_Core* core) {
    uint8_t closureReg = core->codePtr[core->ip++];
    TypeV_Closure* cl = (TypeV_Closure*) core->regs[closureReg].ptr;
    uint8_t offset = cl->offset;

    for(uint8_t i = 0; i < cl->envSize; i++) {
        cl->upvalues[i] = core->funcState->next->regs[i+offset];
    }
}

static inline void coroutine_alloc(TypeV_Core* core) {
    uint8_t destReg = core->codePtr[core->ip++];
    uint8_t closureReg = core->codePtr[core->ip++];
    TypeV_Closure* closure = (TypeV_Closure*) core->regs[closureReg].ptr;

    // allocate the coroutine
    core->regs[destReg].ptr = (uintptr_t) core_coroutine_alloc(core, closure);
    SET_REG_PTR(core->funcState, destReg);
}


static inline void coroutine_fn_alloc(TypeV_Core* core) {
    uint8_t coroutineReg = core->codePtr[core->ip++];
    TypeV_Coroutine* coroutine = (TypeV_Coroutine*) core->regs[coroutineReg].ptr;
    // creates a new function context, if needed, else reuse the old one
    TypeV_FuncState* newState = coroutine->state;

    // we will have to free the .next state of the current function state
    // to avoid memory leaks, because coroutine duplicates the function state
    // when it yields

    newState->prev = core->funcState;
    TypeV_FuncState* originalState = core->funcState->next;

    if (originalState != NULL) {
        if(originalState->next) {
            originalState->next->prev = newState;
        }
        newState->next = originalState->next;
        core_free_function_state(core, originalState);
    }

    core->funcState->next = newState;
}

static inline void coroutine_get_state(TypeV_Core* core) {
    uint8_t destReg = core->codePtr[core->ip++];
    uint8_t coroutineReg = core->codePtr[core->ip++];

    TypeV_Coroutine* coroutine = (TypeV_Coroutine*) core->regs[coroutineReg].ptr;

    core->regs[destReg].u8 = coroutine->executionState;
    CLEAR_REG_PTR(core->funcState, destReg);
}

static inline void coroutine_call(TypeV_Core* core) {
    uint8_t coroutineReg = core->codePtr[core->ip++];
    TypeV_Coroutine* coroutine = (TypeV_Coroutine*) core->regs[coroutineReg].ptr;

    // make sure the coroutine is not finished
    if(coroutine->executionState == TV_COROUTINE_FINISHED) {
        core_panic(core, RT_ERROR_COROUTINE_FINISHED, "Coroutine finished");
    }

    coroutine->executionState = TV_COROUTINE_RUNNING;

    core->activeCoroutine = coroutine;
    coroutine->state->ip = core->ip;

    // back up IP in funcState
    core->funcState->ip = core->ip;

    // point the IP of the coroutine
    core->ip = coroutine->ip;

    // move the function state to the coroutine's function state
    core->funcState = core->funcState->next;

    // set the registers to the coroutine's function state
    core->regs = core->funcState->regs;
}

static inline void coroutine_yield(TypeV_Core* core) {
    TypeV_Coroutine* coroutine = core->activeCoroutine;
    coroutine->executionState = TV_COROUTINE_SUSPENDED;
    coroutine->ip = core->ip;

    core->ip = core->funcState->prev->ip;
    core->funcState = core->funcState->prev;
    core->regs = core->funcState->regs;

    coroutine->state = core_duplicate_function_state(coroutine->state);
    core->activeCoroutine = NULL;
}

static inline void coroutine_ret(TypeV_Core* core) {
    TypeV_Coroutine* coroutine = core->activeCoroutine;
    core->activeCoroutine->executionState = TV_COROUTINE_FINISHED;

    core->ip = core->funcState->prev->ip;
    core->funcState = core->funcState->prev;
    core->regs = core->funcState->regs;

    coroutine->state = core_duplicate_function_state(coroutine->state);
    core->activeCoroutine = NULL;
}

static inline void throw_rt(TypeV_Core* core) {
    uint8_t code = core->codePtr[core->ip++];
    core_panic(core, code, "Runtime error: %s", TypeV_RTErrorMessages[code]);
}

#endif //TYPE_V_INSTRUCTIONS_H
