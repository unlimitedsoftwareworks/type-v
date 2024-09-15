/**
 * Type-V Virtual Machine
 * Author: praisethemoon
 * instructions.h: VM Instructions
 * VM instructions headers are defined here.
 */

#ifndef TYPE_V_INSTRUCTIONS_H
#define TYPE_V_INSTRUCTIONS_H

#include "../core.h"
#include "./opcodes.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "instructions.h"
#include "../core.h"
#include "../utils/utils.h"
#include "../utils/log.h"
#include "../vendor/libtable/table.h"
#include "../stack/stack.h"
#include "../engine.h"


#define CORE_ASSERT(condition, message)


// for aligned data
static inline void typev_memcpy_u64_ptr(unsigned char* dest, const unsigned char* src, size_t n) {

    static void* dispatch_table[] = {&&DO_1, &&DO_2, &&DO_3, &&DO_4, &&DO_5, &&DO_6, &&DO_7, &&DO_8};
    goto *dispatch_table[n - 1];
    DO_8: dest[7] = src[7];
    DO_7: dest[6] = src[6];
    DO_6: dest[5] = src[5];
    DO_5: dest[4] = src[4];
    DO_4: dest[3] = src[3];
    DO_3: dest[2] = src[2];
    DO_2: dest[1] = src[1];
    DO_1: dest[0] = src[0];
}


// Custom memcpy function
static inline uint64_t typev_memcpy_u64(const unsigned char *s, size_t size) {
    uint64_t dest = 0;

    for (size_t i = 0; i < size; ++i) {
        dest |= (uint64_t)s[i] << (i * 8);
    }

    return dest;
}

static inline void typev_memcpy_u64_ptr_u64_ptr(const unsigned char* dest, const unsigned char *s, size_t size) {
    for (size_t i = 0; i < size; ++i) {
        *(uint64_t*)dest |= (uint64_t)s[i] << (i * 8);
    }
}

static inline void mv_reg_reg(TypeV_Core* core){
    const uint8_t target = core->codePtr[core->ip++];
    const uint8_t source = core->codePtr[core->ip++];

    core->ip++;
    //uint8_t byteSize = core->codePtr[core->ip++];
    CORE_ASSERT(isValidByte(byteSize), "Invalid byte size");

    //typev_memcpy_u64_ptr(&core->regs[target], &core->regs[source], byteSize);
    core->regs[target].ptr = core->regs[source].ptr;
}

static inline void mv_reg_reg_ptr(TypeV_Core* core){
    const uint8_t target = core->codePtr[core->ip++];
    const uint8_t source = core->codePtr[core->ip++];

    typev_memcpy_u64_ptr(&core->regs[target], &core->regs[source], PTR_SIZE);
}

static inline void mv_reg_null(TypeV_Core* core){
    const uint8_t target = core->codePtr[core->ip++];
    core->regs[target].ptr = 0;
}

static inline void mv_reg_i(TypeV_Core* core){
    const uint8_t target = core->codePtr[core->ip++];
    uint8_t immediate_size = core->codePtr[core->ip++];
    //uint64_t immediate = 0;
    uint64_t immediate = typev_memcpy_u64(&core->codePtr[core->ip], immediate_size);
    // data could be unaligned
    //typev_memcpy_u64_ptr(&immediate, &core->codePtr[core->ip], immediate_size);
    core->ip += immediate_size;
    core->regs[target].ptr =  immediate;
    //typev_memcpy_u64_ptr(&core->regs[target], &immediate, immediate_size);
    //typev_memcpy_u64_ptr(&core->regs[target], &immediate, 8);
}


static inline void mv_reg_const(TypeV_Core* core){
    const uint8_t target = core->codePtr[core->ip++];
    const uint8_t offset_length = core->codePtr[core->ip++];
    size_t constant_offset = 0;
    // data could be unaligned
    typev_memcpy_u64_ptr(&constant_offset, &core->codePtr[core->ip],  offset_length);
    core->ip += offset_length;

    uint8_t byteSize = core->codePtr[core->ip++];

    CORE_ASSERT(isValidByte(byteSize), "Invalid byte size");

    typev_memcpy_u64_ptr(&core->regs[target], &core->constPtr[constant_offset], byteSize);
}

/*
static inline void mv_reg_const_8(TypeV_Core* core){
    const uint8_t target = core->codePtr[core->ip++];

    const uint8_t offset_length = core->codePtr[core->ip++];
    size_t constant_offset = 0;
    // data could be unaligned
    typev_memcpy_u64_ptr(&constant_offset, &core->codePtr[core->ip],  offset_length);
    core->ip += offset_length;
    typev_memcpy_u64_ptr(&core->regs[target], &core->constPtr[constant_offset], 1);
}

static inline void mv_reg_const_16(TypeV_Core* core){
    const uint8_t target = core->codePtr[core->ip++];

    const uint8_t offset_length = core->codePtr[core->ip++];
    size_t constant_offset = 0;
    // data could be unaligned
    typev_memcpy_u64_ptr(&constant_offset, &core->codePtr[core->ip],  offset_length);
    core->ip += offset_length;
    typev_memcpy_u64_ptr(&core->regs[target], &core->constPtr[constant_offset], 2);
}

static inline void mv_reg_const_32(TypeV_Core* core){
    const uint8_t target = core->codePtr[core->ip++];

    const uint8_t offset_length = core->codePtr[core->ip++];
    size_t constant_offset = 0;
    // data could be unaligned
    typev_memcpy_u64_ptr(&constant_offset, &core->codePtr[core->ip],  offset_length);
    core->ip += offset_length;
    typev_memcpy_u64_ptr(&core->regs[target], &core->constPtr[constant_offset], 4);
}

static inline void mv_reg_const_64(TypeV_Core* core){
    const uint8_t target = core->codePtr[core->ip++];

    const uint8_t offset_length = core->codePtr[core->ip++];
    size_t constant_offset = 0;
    // data could be unaligned
    typev_memcpy_u64_ptr(&constant_offset, &core->codePtr[core->ip],  offset_length);
    core->ip += offset_length;
    typev_memcpy_u64_ptr(&core->regs[target], &core->constPtr[constant_offset], 8);
}
*/

static inline void mv_reg_const_ptr(TypeV_Core* core){
    const uint8_t target = core->codePtr[core->ip++];

    const uint8_t offset_length = core->codePtr[core->ip++];
    size_t constant_offset = 0;
    // data could be unaligned
    typev_memcpy_u64_ptr(&constant_offset, &core->codePtr[core->ip],  offset_length);
    core->ip += offset_length;
    typev_memcpy_u64_ptr(&core->regs[target], &core->constPtr[constant_offset], PTR_SIZE);
}

static inline void mv_global_reg(TypeV_Core* core){
    const uint8_t offset_length = core->codePtr[core->ip++];
    size_t offset = 0;
    typev_memcpy_u64_ptr(&offset, &core->codePtr[core->ip],  offset_length);
    core->ip += offset_length;
    const uint8_t source = core->codePtr[core->ip++];
    uint8_t byteSize = core->codePtr[core->ip++];

    CORE_ASSERT(isValidByte(byteSize), "Invalid byte size");

    typev_memcpy_u64_ptr(&core->globalPtr[offset], &core->regs[source], byteSize);
}

static inline void mv_global_reg_ptr(TypeV_Core* core){
    const uint8_t offset_length = core->codePtr[core->ip++];
    size_t offset = 0;
    typev_memcpy_u64_ptr(&offset, &core->codePtr[core->ip],  offset_length);
    core->ip += offset_length;
    const uint8_t source = core->codePtr[core->ip++];

    typev_memcpy_u64_ptr(&core->globalPtr[offset], &core->regs[source], PTR_SIZE);
}

static inline void mv_reg_global(TypeV_Core* core){
    const uint8_t target = core->codePtr[core->ip++];
    const uint8_t offset_length = core->codePtr[core->ip++];
    size_t offset = 0;
    typev_memcpy_u64_ptr(&offset, &core->codePtr[core->ip],  offset_length);
    core->ip += offset_length;
    uint8_t byteSize = core->codePtr[core->ip++];
    CORE_ASSERT(isValidByte(byteSize), "Invalid byte size");

    typev_memcpy_u64_ptr(&core->regs[target], &core->globalPtr[offset], byteSize);
}

static inline void mv_reg_global_ptr(TypeV_Core* core){
    const uint8_t target = core->codePtr[core->ip++];
    const uint8_t offset_length = core->codePtr[core->ip++];
    size_t offset = 0;
    typev_memcpy_u64_ptr(&offset, &core->codePtr[core->ip],  offset_length);
    core->ip += offset_length;


    typev_memcpy_u64_ptr(&core->regs[target], &core->globalPtr[offset], PTR_SIZE);
}

static inline void s_alloc(TypeV_Core* core){
    const uint8_t dest_reg = core->codePtr[core->ip++];
    ASSERT(dest_reg < MAX_REG, "Invalid register index");
    const uint8_t fields_count = core->codePtr[core->ip++];
    uint16_t struct_size = 0;
    typev_memcpy_u64_ptr(&struct_size, &core->codePtr[core->ip],  2);
    core->ip += 2;

    // allocate memory for struct
    size_t mem = core_struct_alloc(core, fields_count, struct_size);
    // move the pointer to R16
    core->regs[dest_reg].ptr = mem;
}

static inline void s_reg_field(TypeV_Core* core){
    const uint8_t src_reg = core->codePtr[core->ip++];
    ASSERT(src_reg < MAX_REG, "Invalid register index");

    uint8_t field_index = core->codePtr[core->ip++];

    uint32_t globalFieldIndex = 0;
    typev_memcpy_u64_ptr(&globalFieldIndex, &core->codePtr[core->ip],  4);
    core->ip += 4;

    uint16_t offset = 0;
    typev_memcpy_u64_ptr(&offset, &core->codePtr[core->ip],  2);
    core->ip += 2;


    TypeV_Struct* struct_ptr = (TypeV_Struct*)core->regs[src_reg].ptr;
    struct_ptr->globalFields[field_index] = globalFieldIndex;
    struct_ptr->fieldOffsets[field_index] = offset;
}



static inline void s_loadf(TypeV_Core* core){
    uint32_t ip_start = core->ip;

    const uint8_t target = core->codePtr[core->ip++];
    const uint8_t source = core->codePtr[core->ip++];
    uint32_t field_index = 0;
    typev_memcpy_u64_ptr(&field_index, &core->codePtr[core->ip],  4);

    core->ip += 4;

    uint8_t byteSize = core->codePtr[core->ip++];
    CORE_ASSERT(isValidByte(byteSize), "Invalid byte size");

    TypeV_Struct* struct_ptr = (TypeV_Struct*)core->regs[source].ptr;
    //TypeV_ObjectHeader *header = (TypeV_ObjectHeader *)core->regs[source].ptr;

    uint8_t index = object_find_global_index(core, struct_ptr->globalFields, struct_ptr->numFields, field_index);


    typev_memcpy_u64_ptr(&core->regs[target], struct_ptr->dataPointer+struct_ptr->fieldOffsets[index], byteSize);
}

static inline void s_loadf_ptr(TypeV_Core* core){
    const uint8_t target = core->codePtr[core->ip++];

    const uint8_t source = core->codePtr[core->ip++];

    uint32_t field_index = 0;
    typev_memcpy_u64_ptr(&field_index, &core->codePtr[core->ip],  4);
    core->ip += 4;

    TypeV_Struct* struct_ptr = (TypeV_Struct*)core->regs[source].ptr;
    uint8_t index = object_find_global_index(core, struct_ptr->globalFields, struct_ptr->numFields, field_index);
    typev_memcpy_u64_ptr(&core->regs[target], struct_ptr->dataPointer+struct_ptr->fieldOffsets[index], PTR_SIZE);
}

static inline void s_storef_const(TypeV_Core* core){
    const uint8_t dest_reg = core->codePtr[core->ip++];
    uint32_t field_index = 0;
    typev_memcpy_u64_ptr(&field_index, &core->codePtr[core->ip],  4);
    core->ip += 4;

    size_t offset = 0;
    typev_memcpy_u64_ptr(&offset, &core->codePtr[core->ip], 8);
    core->ip += 8;
    uint8_t byteSize = core->codePtr[core->ip++];
    CORE_ASSERT(isValidByte(byteSize), "Invalid byte size");
    TypeV_Struct* struct_ptr = (TypeV_Struct*)core->regs[dest_reg].ptr;
    uint8_t index = object_find_global_index(core, struct_ptr->globalFields, struct_ptr->numFields, field_index);
    typev_memcpy_u64_ptr(struct_ptr->dataPointer+struct_ptr->fieldOffsets[index], &core->constPtr[offset], byteSize);
}

static inline void s_storef_const_ptr(TypeV_Core* core){
    const uint8_t dest_reg = core->codePtr[core->ip++];

    uint32_t field_index = 0;
    typev_memcpy_u64_ptr(&field_index, &core->codePtr[core->ip],  4);
    core->ip += 4;

    size_t offset = 0;
    typev_memcpy_u64_ptr(&offset, &core->codePtr[core->ip], 8);
    core->ip += 8;
    TypeV_Struct* struct_ptr = (TypeV_Struct*)core->regs[dest_reg].ptr;
    uint8_t index = object_find_global_index(core, struct_ptr->globalFields, struct_ptr->numFields, field_index);
    typev_memcpy_u64_ptr(struct_ptr->dataPointer+struct_ptr->fieldOffsets[index], &core->constPtr[offset], PTR_SIZE);
}

static inline void s_storef_reg(TypeV_Core* core){
    const uint8_t dest_reg = core->codePtr[core->ip++];

    uint32_t field_index = 0;
    typev_memcpy_u64_ptr(&field_index, &core->codePtr[core->ip],  4);
    core->ip += 4;

    ASSERT(field_index < MAX_REG, "Invalid register index");
    const uint8_t source = core->codePtr[core->ip++];
    uint8_t byteSize = core->codePtr[core->ip++];
    CORE_ASSERT(isValidByte(byteSize), "Invalid byte size");

    TypeV_Struct *struct_ptr = (TypeV_Struct *) core->regs[dest_reg].ptr;
    uint8_t index = object_find_global_index(core, struct_ptr->globalFields, struct_ptr->numFields, field_index);
    typev_memcpy_u64_ptr(struct_ptr->dataPointer + struct_ptr->fieldOffsets[index], &core->regs[source], byteSize);
}

static inline void s_storef_reg_ptr(TypeV_Core* core){
    const uint8_t dest_reg = core->codePtr[core->ip++];

    uint32_t field_index = 0;
    typev_memcpy_u64_ptr(&field_index, &core->codePtr[core->ip],  4);
    core->ip += 4;

    ASSERT(field_index < MAX_REG, "Invalid register index");
    const uint8_t source = core->codePtr[core->ip++];

    TypeV_Struct *struct_ptr = (TypeV_Struct *) core->regs[dest_reg].ptr;
    uint8_t index = object_find_global_index(core, struct_ptr->globalFields, struct_ptr->numFields, field_index);
    typev_memcpy_u64_ptr(struct_ptr->dataPointer + struct_ptr->fieldOffsets[index], &core->regs[source], 8);
    core_gc_update_struct_field(core, struct_ptr, (void*)core->regs[source].ptr, index);
}

static inline void c_alloc(TypeV_Core* core){
    const uint8_t dest_reg = core->codePtr[core->ip++];
    ASSERT(dest_reg < MAX_REG, "Invalid register index");
    const uint8_t methods_count = core->codePtr[core->ip++];
    size_t fields_size = 0;
    typev_memcpy_u64_ptr(&fields_size, &core->codePtr[core->ip],  2);
    core->ip += 2;

    uint8_t classId_size = core->codePtr[core->ip++];
    uint64_t classId = 0;
    typev_memcpy_u64_ptr(&classId, &core->codePtr[core->ip],  classId_size);
    core->ip += classId_size;


    // allocate memory for class
    size_t mem = core_class_alloc(core, methods_count, fields_size, classId);
    // move the pointer to R17
    core->regs[dest_reg].ptr = mem;
}

static inline void c_storem(TypeV_Core* core){
    const uint8_t dest_reg = core->codePtr[core->ip++];
    ASSERT(dest_reg < MAX_REG, "Invalid register index");
    const uint8_t local_method_index = core->codePtr[core->ip++];
    uint32_t global_method_index = 0;
    typev_memcpy_u64_ptr(&global_method_index, &core->codePtr[core->ip],  4);
    core->ip += 4;

    size_t method_address = 0; /* we do not increment method_address here*/
    typev_memcpy_u64_ptr(&method_address, &core->codePtr[core->ip], 8);
    core->ip += 8;

    TypeV_Class* c = (TypeV_Class*)core->regs[dest_reg].ptr;
    //LOG_INFO("Storing method %d at method_address %d in class %p", method_index, method_address, (void*)c);
    c->globalMethods[local_method_index] = global_method_index;
    c->methods[local_method_index] = method_address;
}

static inline void c_loadm(TypeV_Core* core){
    const uint8_t target = core->codePtr[core->ip++];
    const uint8_t class_reg = core->codePtr[core->ip++];
    uint32_t method_index = 0;

    typev_memcpy_u64_ptr(&method_index, &core->codePtr[core->ip],  4);
    core->ip += 4;

    TypeV_Class* c = (TypeV_Class*)core->regs[class_reg].ptr;

    //uint8_t idx = class_find_global_index(c, method_index);
    uint8_t idx = object_find_global_index(core, c->globalMethods, c->numMethods, method_index);

    LOG_INFO("Loading method %d from class %p", method_index, (void*)c);

    size_t offset = c->methods[idx];
    core->regs[target].ptr = offset;
}

static inline void c_storef_reg(TypeV_Core* core){
    const uint8_t class_reg = core->codePtr[core->ip++];
    size_t field_offset = 0;
    typev_memcpy_u64_ptr(&field_offset, &core->codePtr[core->ip], 2);
    core->ip += 2;
    const uint8_t source = core->codePtr[core->ip++];
    uint8_t byteSize = core->codePtr[core->ip++];
    CORE_ASSERT(isValidByte(byteSize), "Invalid byte size");

    TypeV_Class* c = (TypeV_Class*)core->regs[class_reg].ptr;
    typev_memcpy_u64_ptr(c->data+field_offset, &core->regs[source], byteSize);
}

static inline void c_storef_reg_ptr(TypeV_Core* core){
    const uint8_t class_reg = core->codePtr[core->ip++];
    ASSERT(class_reg < MAX_REG, "Invalid register index");
    size_t field_offset = 0;
    typev_memcpy_u64_ptr(&field_offset, &core->codePtr[core->ip], 2);
    core->ip += 2;
    const uint8_t source = core->codePtr[core->ip++];

    TypeV_Class* c = (TypeV_Class*)core->regs[class_reg].ptr;
    typev_memcpy_u64_ptr(c->data+field_offset, &core->regs[source].ptr, PTR_SIZE);

    core_gc_update_class_field(core, c, (void*)core->regs[source].ptr, field_offset);
}

static inline void c_storef_const(TypeV_Core* core) {
    const uint8_t class_reg = core->codePtr[core->ip++];
    ASSERT(class_reg < MAX_REG, "Invalid register index");
    size_t field_offset = 0;
    typev_memcpy_u64_ptr(&field_offset, &core->codePtr[core->ip], 2);
    core->ip += 2;
    size_t offset = 0;
    typev_memcpy_u64_ptr(&offset, &core->codePtr[core->ip], 8);
    core->ip += 8;
    uint8_t byteSize = core->codePtr[core->ip++];
    CORE_ASSERT(isValidByte(byteSize), "Invalid byte size");

    TypeV_Class *c = (TypeV_Class *) core->regs[class_reg].ptr;
    typev_memcpy_u64_ptr(c->data + field_offset, &core->constPtr[offset], byteSize);
}

static inline void c_storef_const_ptr(TypeV_Core* core){
    const uint8_t class_reg = core->codePtr[core->ip++];
    ASSERT(class_reg < MAX_REG, "Invalid register index");
    size_t field_offset = 0;
    typev_memcpy_u64_ptr(&field_offset, &core->codePtr[core->ip], 2);
    core->ip += 2;
    size_t offset = 0;
    typev_memcpy_u64_ptr(&offset, &core->codePtr[core->ip], 8);
    core->ip += 8;

    TypeV_Class* c = (TypeV_Class*)core->regs[class_reg].ptr;
    typev_memcpy_u64_ptr(c->data+field_offset, &core->constPtr[offset], PTR_SIZE);
}

static inline void c_loadf(TypeV_Core* core){
    const uint8_t target = core->codePtr[core->ip++];
    const uint8_t class_reg = core->codePtr[core->ip++];
    size_t offset = 0;
    typev_memcpy_u64_ptr(&offset, &core->codePtr[core->ip], 2);
    core->ip += 2;
    uint8_t byteSize = core->codePtr[core->ip++];
    CORE_ASSERT(isValidByte(byteSize), "Invalid byte size");
    TypeV_Class* c = (TypeV_Class*)core->regs[class_reg].ptr;
    typev_memcpy_u64_ptr(&core->regs[target], c->data+offset, byteSize);
}

static inline void c_loadf_ptr(TypeV_Core* core){
    const uint8_t target = core->codePtr[core->ip++];
    const uint8_t class_reg = core->codePtr[core->ip++];
    size_t offset = 0;
    typev_memcpy_u64_ptr(&offset, &core->codePtr[core->ip], 2);
    core->ip += 2;
    TypeV_Class* c = (TypeV_Class*)core->regs[class_reg].ptr;
    typev_memcpy_u64_ptr(&core->regs[target], c->data+offset, PTR_SIZE);
}

static inline void i_is_c(TypeV_Core* core){
    const uint8_t target = core->codePtr[core->ip++];
    const uint8_t interface_reg = core->codePtr[core->ip++];

    uint64_t classId = 0;
    typev_memcpy_u64_ptr(&classId, &core->codePtr[core->ip],  8);
    core->ip += 8;

    TypeV_Class* class_ = (TypeV_Class*)core->regs[interface_reg].ptr;

    core->regs[target].u8 = class_->uid == classId;
}

static inline void i_has_m(TypeV_Core* core){
    uint32_t lookUpMethodId = 0;
    typev_memcpy_u64_ptr(&lookUpMethodId, &core->codePtr[core->ip],  4);
    core->ip += 4;

    const uint8_t interface_reg = core->codePtr[core->ip++];

    size_t offset = 0;
    typev_memcpy_u64_ptr(&offset, &core->codePtr[core->ip], 8);
    core->ip += 8;

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

    size_t num_elements = 0;
    typev_memcpy_u64_ptr(&num_elements, &core->codePtr[core->ip], 8);
    core->ip += 8;
    // next read the element size
    uint8_t element_size = core->codePtr[core->ip++];
    if(element_size > 8) {
        core_panic(core, 1, "Element size too large %d", element_size);
    }

    // allocate memory for struct
    size_t mem = core_array_alloc(core, num_elements, element_size);
    // move the pointer to R19
    core->regs[dest].ptr = mem;
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
    typev_memcpy_u64_ptr(array->data+(core->regs[index].u64*array->elementSize), &core->regs[source], byteSize);
}

static inline void a_storef_reg_ptr(TypeV_Core* core){
    const uint8_t array_reg = core->codePtr[core->ip++];
    const uint8_t index = core->codePtr[core->ip++];
    const uint8_t source = core->codePtr[core->ip++];

    TypeV_Array* array = (TypeV_Array*)core->regs[array_reg].ptr;

    if(core->regs[index].u64 >= array->length) {
        core_panic(core, 1, "Index out of bounds %d >= %d", core->regs[index].u64, array->length);
    }
    typev_memcpy_u64_ptr(array->data+(core->regs[index].u64*array->elementSize), &core->regs[source], PTR_SIZE);
    core_gc_update_array_field(core, array, (void*)core->regs[source].ptr, core->regs[index].u64);
}

static inline void a_storef_const(TypeV_Core* core){
    const uint8_t array_reg = core->codePtr[core->ip++];
    uint8_t indexReg = core->codePtr[core->ip++];
    size_t offset = 0;
    typev_memcpy_u64_ptr(&offset, &core->codePtr[core->ip], 8);
    core->ip += 8;

    uint8_t byteSize = core->codePtr[core->ip++];
    CORE_ASSERT(isValidByte(byteSize), "Invalid byte size");

    TypeV_Array* array = (TypeV_Array*)core->regs[array_reg].ptr;
    ASSERT(core->regs[indexReg].u64 < array->length, "Index out of bounds");
    typev_memcpy_u64_ptr(array->data+(core->regs[indexReg].u64*array->elementSize), &core->constPtr[offset], byteSize);
}

static inline void a_storef_const_ptr(TypeV_Core* core){
    const uint8_t array_reg = core->codePtr[core->ip++];
    uint8_t indexReg = core->codePtr[core->ip++];
    size_t offset = 0;
    typev_memcpy_u64_ptr(&offset, &core->codePtr[core->ip], 8);
    core->ip += 8;
    TypeV_Array* array = (TypeV_Array*)core->regs[array_reg].ptr;
    ASSERT(core->regs[indexReg].u64 < array->length, "Index out of bounds");
    typev_memcpy_u64_ptr(array->data+(core->regs[indexReg].u64*array->elementSize), &core->constPtr[offset], PTR_SIZE);
}

static inline void a_loadf(TypeV_Core* core) {
    const uint8_t target = core->codePtr[core->ip++];
    const uint8_t index = core->codePtr[core->ip++];
    const uint8_t array_reg = core->codePtr[core->ip++];

    TypeV_Array* array = (TypeV_Array*)core->regs[array_reg].ptr;

    uint64_t idx = core->regs[index].u64;

    if(idx >= array->length) {
        core_panic(core, 1, "Index out of bounds %d >= %d", idx, array->length);
    }


    uint8_t byteSize = core->codePtr[core->ip++];
    CORE_ASSERT(isValidByte(byteSize), "Invalid byte size");

    typev_memcpy_u64_ptr(&core->regs[target], array->data + (idx * array->elementSize), byteSize);
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

    typev_memcpy_u64_ptr(&core->regs[target], array->data + (core->regs[index].u64 * array->elementSize), PTR_SIZE);
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
    typev_memcpy_u64_ptr(&offset, &core->codePtr[core->ip],  offset_length);
    core->ip += offset_length;

    // get bytes
    uint8_t bytesize = core->codePtr[core->ip++];
    if(bytesize == 0) bytesize = PTR_SIZE;
    ASSERT(bytesize <= 8, "Invalid byte size");

    switch (bytesize) {
        case 0:
            stack_push_ptr(core, (size_t)&core->constPtr[offset]);
            break;
        case 1:
            stack_push_8(core, core->constPtr[offset]);
            break;
        case 2:
            stack_push_16(core, *((uint16_t*)&core->constPtr[offset]));
            break;
        case 4:
            stack_push_32(core, *((uint32_t*)&core->constPtr[offset]));
            break;
        case 8:
            stack_push_64(core, *((uint64_t*)&core->constPtr[offset]));
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
    stack_pop_ptr(core, &core->regs[target].ptr);
}

static inline void fn_alloc(TypeV_Core* core){
    // creates a new function context, if needed, else reuse the old one
    TypeV_FuncState* newState;
    if(core->funcState->next == NULL) {
        newState = core_create_function_state(core->funcState);
    }
    else {
        newState = core->funcState->next;
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

    //typev_memcpy_u64_ptr(&core->funcState->next->regs[dest_reg], &core->regs[source_reg], byteSize);
    core->funcState->next->regs[dest_reg] = core->regs[source_reg];
}

static inline void fn_set_reg_ptr(TypeV_Core* core){
    const uint8_t dest_reg = core->codePtr[core->ip++];
    const uint8_t source_reg = core->codePtr[core->ip++];

    core->funcState->next->regs[dest_reg] = core->regs[source_reg];
}

static inline void fn_call(TypeV_Core* core){
    // get the register
    const uint8_t target = core->codePtr[core->ip++];
    const size_t adr = core->regs[target].ptr;

    // back up IP in funcState
    core->funcState->ip = core->ip;
    core->ip = adr;
    core->funcState = core->funcState->next;
    core->regs = core->funcState->regs;
}

static inline void fn_calli(TypeV_Core* core){
    // same as fn_call but takes immediate value
    core->ip++;
    //const uint8_t offset_length = core->codePtr[core->ip++];
    size_t offset = typev_memcpy_u64(&core->codePtr[core->ip], 8);
    //typev_memcpy_u64_ptr(&offset, &core->codePtr[core->ip],  offset_length);
    core->ip += 8;


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
}

static inline void fn_get_ret_reg_ptr(TypeV_Core* core){
    const uint8_t dest_reg = core->codePtr[core->ip++];
    const uint8_t source_reg = core->codePtr[core->ip++];

    // we grab the register from the .next into the current one
    core->regs[dest_reg].ptr = core->funcState->next->regs[source_reg].ptr;
}

#define OP_CAST(d1, d2, type) \
static inline void cast_##d1##_##d2(TypeV_Core* core){ \
    uint8_t op1 = core->codePtr[core->ip++];\
    core->regs[op1].d2 = (type) core->regs[op1].d1;\
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
    typev_memcpy_u64_ptr(&value, &core->regs[reg], from);

    // Perform sign extension without branching
    if (from < 8) {
        int64_t signExtMask = (value & (1LL << (from * 8 - 1))) ? (-1LL << (from * 8)) : 0;
        value |= signExtMask;
    }

    // Store the result back in the register
    typev_memcpy_u64_ptr(&core->regs[reg], &value, to);
}

static inline void upcast_u(TypeV_Core* core) {
    uint8_t reg = core->codePtr[core->ip++];
    uint8_t from = core->codePtr[core->ip++];
    uint8_t to = core->codePtr[core->ip++];

    ASSERT(reg < MAX_REG, "Invalid register index");
    ASSERT(from <= 8 && to <= 8 && from < to, "Invalid byte sizes for upcasting");

    // Extract the value from the register
    uint64_t value = 0;
    typev_memcpy_u64_ptr(&value, &core->regs[reg], from);

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
    //typev_memcpy_u64_ptr(&core->regs[reg], &value, to);
}

static inline void upcast_f(TypeV_Core* core) {
    uint8_t reg = core->codePtr[core->ip++];
    uint8_t from = core->codePtr[core->ip++];
    uint8_t to = core->codePtr[core->ip++];

    ASSERT(reg < MAX_REG, "Invalid register index");
    ASSERT((from == 4 && to == 8), "Invalid byte sizes for floating-point upcasting");

    // Extract the float value from the register
    float floatValue = 0.0f;
    typev_memcpy_u64_ptr(&floatValue, &core->regs[reg], from);

    // Convert to double
    double doubleValue = (double)floatValue;

    // Store the result back in the register
    typev_memcpy_u64_ptr(&core->regs[reg], &doubleValue, to);
}


static inline void dcast_i(TypeV_Core* core) {
    uint8_t reg = core->codePtr[core->ip++];
    uint8_t from = core->codePtr[core->ip++];
    uint8_t to = core->codePtr[core->ip++];

    ASSERT(reg < MAX_REG, "Invalid register index");
    ASSERT(from <= 8 && to <= 8 && from > to, "Invalid byte sizes for downcasting");

    // We simply need to copy the lower 'to' bytes from the register
    int64_t value = 0;
    typev_memcpy_u64_ptr(&value, &core->regs[reg], to);

    // Store the result back in the register (overwriting only 'to' bytes)
    typev_memcpy_u64_ptr(&core->regs[reg], &value, to);
}

static inline void dcast_u(TypeV_Core* core) {
    uint8_t reg = core->codePtr[core->ip++];
    uint8_t from = core->codePtr[core->ip++];
    uint8_t to = core->codePtr[core->ip++];

    ASSERT(reg < MAX_REG, "Invalid register index");
    ASSERT(from <= 8 && to <= 8 && from > to, "Invalid byte sizes for downcasting");

    // Extract the value from the register
    uint64_t value = 0;
    typev_memcpy_u64_ptr(&value, &core->regs[reg], from);

    // Truncate the value to the smaller size
    uint64_t truncatedValue = 0;
    typev_memcpy_u64_ptr(&truncatedValue, &value, to);

    // Store the truncated value back in the register
    typev_memcpy_u64_ptr(&core->regs[reg], &truncatedValue, to);
}

static inline void dcast_f(TypeV_Core* core) {
    uint8_t reg = core->codePtr[core->ip++];
    uint8_t from = core->codePtr[core->ip++];
    uint8_t to = core->codePtr[core->ip++];

    ASSERT(reg < MAX_REG, "Invalid register index");
    ASSERT((from == 8 && to == 4), "Invalid byte sizes for floating-point downcasting");

    // Extract the double value from the register
    double doubleValue = 0.0;
    typev_memcpy_u64_ptr(&doubleValue, &core->regs[reg], from);

    // Convert to float
    float floatValue = (float)doubleValue;

    // Store the result back in the register
    typev_memcpy_u64_ptr(&core->regs[reg], &floatValue, to);
}



#define OP_BINARY(name, type, op)\
static inline void name##_##type(TypeV_Core* core){\
    uint8_t target = core->codePtr[core->ip++];\
    uint8_t op1 = core->codePtr[core->ip++];\
    uint8_t op2 = core->codePtr[core->ip++];\
    core->regs[target].type = core->regs[op1].type op core->regs[op2].type;\
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
}

OP_BINARY(add, i64, +)
//OP_BINARY(add, u64, +)
OP_BINARY(add, f32, +)
OP_BINARY(add, f64, +)


static inline void add_u64(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];
    uint8_t op2 = core->codePtr[core->ip++];
    core->regs[target].u64 = core->regs[op1].u64 + core->regs[op2].u64;
}

static inline void add_ptr_u8(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];
    uint8_t op2 = core->codePtr[core->ip++];
    core->regs[target].ptr = core->regs[op1].ptr + core->regs[op2].u8;
}

static inline void add_ptr_u16(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];
    uint8_t op2 = core->codePtr[core->ip++];
    core->regs[target].ptr = core->regs[op1].ptr + core->regs[op2].u16;
}

static inline void add_ptr_u32(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];
    uint8_t op2 = core->codePtr[core->ip++];
    core->regs[target].ptr = core->regs[op1].ptr + core->regs[op2].u32;
}

static inline void add_ptr_u64(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];
    uint8_t op2 = core->codePtr[core->ip++];
    core->regs[target].ptr = core->regs[op1].ptr + core->regs[op2].u64;
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
}

static inline void sub_ptr_u16(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];
    uint8_t op2 = core->codePtr[core->ip++];
    core->regs[target].ptr = core->regs[op1].ptr - core->regs[op2].u16;
}

static inline void sub_ptr_u32(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];
    uint8_t op2 = core->codePtr[core->ip++];
    core->regs[target].ptr = core->regs[op1].ptr - core->regs[op2].u32;
}

static inline void sub_ptr_u64(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];
    uint8_t op2 = core->codePtr[core->ip++];
    core->regs[target].ptr = core->regs[op1].ptr - core->regs[op2].u64;
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
}

static inline void band_16(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];
    uint8_t op2 = core->codePtr[core->ip++];

    core->regs[target].u16 = core->regs[op1].u16 & core->regs[op2].u16;
}

static inline void band_32(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];
    uint8_t op2 = core->codePtr[core->ip++];

    core->regs[target].u32 = core->regs[op1].u32 & core->regs[op2].u32;
}

static inline void band_64(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];
    uint8_t op2 = core->codePtr[core->ip++];

    core->regs[target].u64 = core->regs[op1].u64 & core->regs[op2].u64;
}

static inline void bor_8(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];
    uint8_t op2 = core->codePtr[core->ip++];

    core->regs[target].u8 = core->regs[op1].u8 | core->regs[op2].u8;
}

static inline void bor_16(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];
    uint8_t op2 = core->codePtr[core->ip++];

    core->regs[target].u16 = core->regs[op1].u16 | core->regs[op2].u16;
}

static inline void bor_32(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];
    uint8_t op2 = core->codePtr[core->ip++];

    core->regs[target].u32 = core->regs[op1].u32 | core->regs[op2].u32;
}

static inline void bor_64(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];
    uint8_t op2 = core->codePtr[core->ip++];

    core->regs[target].u64 = core->regs[op1].u64 | core->regs[op2].u64;
}

static inline void bxor_8(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];

    core->regs[target].u8 = core->regs[op1].u8 ^ core->regs[target].u8;
}

static inline void bxor_16(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];

    core->regs[target].u16 = core->regs[op1].u16 ^ core->regs[target].u16;
}

static inline void bxor_32(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];

    core->regs[target].u32 = core->regs[op1].u32 ^ core->regs[target].u32;
}

static inline void bxor_64(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];

    core->regs[target].u64 = core->regs[op1].u64 ^ core->regs[target].u64;
}

static inline void bnot_8(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];

    core->regs[target].u8 = ~core->regs[op1].u8;
}

static inline void bnot_16(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];

    core->regs[target].u16 = ~core->regs[op1].u16;
}

static inline void bnot_32(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];

    core->regs[target].u32 = ~core->regs[op1].u32;
}

static inline void bnot_64(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];

    core->regs[target].u64 = ~core->regs[op1].u64;
}

static inline void and(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];
    uint8_t op2 = core->codePtr[core->ip++];

    core->regs[target].u8 = core->regs[op1].u8 && core->regs[op2].u8;
}

static inline void or(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];
    uint8_t op2 = core->codePtr[core->ip++];

    core->regs[target].u8 = core->regs[op1].u8 || core->regs[op2].u8;
}

static inline void not(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];

    core->regs[target].u8 = !core->regs[op1].u8;
}

static inline void jmp(TypeV_Core* core){
    core->ip++;
    //const uint8_t offset_length = core->codePtr[core->ip++];
    size_t offset = typev_memcpy_u64(&core->codePtr[core->ip], 8);\
    core->ip = offset;
}


#define OP_CMP(name, type)\
static inline void j_cmp_##name(TypeV_Core* core) {\
    uint8_t op1 = core->codePtr[core->ip++];\
    uint8_t op2 = core->codePtr[core->ip++];\
    uint8_t cmpType = core->codePtr[core->ip++];\
    size_t offset = typev_memcpy_u64(&core->codePtr[core->ip], 8);\
    core->ip += 8;\
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
            core_panic(core, -1, "Invalid comparison type");\
            exit(-1);\
    }\
}\


//OP_CMP(u8, uint8_t)

static inline void j_cmp_u8(TypeV_Core* core) {
    uint8_t op1 = core->codePtr[core->ip++];
    uint8_t op2 = core->codePtr[core->ip++];
    uint8_t cmpType = core->codePtr[core->ip++];
    size_t offset = typev_memcpy_u64(&core->codePtr[core->ip], 8);
    core->ip += 8;
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
            core_panic(core, -1, "Invalid comparison type");\
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
    size_t offset = typev_memcpy_u64(&core->codePtr[core->ip], 8);
    core->ip += 8;
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
            core_panic(core, -1, "Invalid comparison type");\
            exit(-1);
    }
}

OP_CMP(i32, int32_t)

static inline void j_cmp_u64(TypeV_Core* core) {
    uint8_t op1 = core->codePtr[core->ip++];
    uint8_t op2 = core->codePtr[core->ip++];
    uint8_t cmpType = core->codePtr[core->ip++];
    size_t offset = typev_memcpy_u64(&core->codePtr[core->ip], 8);
    core->ip += 8;
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
            core_panic(core, -1, "Invalid comparison type");\
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
    size_t offset = typev_memcpy_u64(&core->codePtr[core->ip], 8);
    core->ip += 8;
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
            core_panic(core, -1, "Invalid comparison type");\
            exit(-1);
    }
}

#undef OP_CMP


static inline void j_cmp_bool(TypeV_Core* core) {
    uint8_t op1 = core->codePtr[core->ip++];
    uint8_t op2 = core->codePtr[core->ip++];
    uint8_t cmpType = core->codePtr[core->ip++];
    size_t offset = typev_memcpy_u64(&core->codePtr[core->ip], 8);
    core->ip += 8;
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
        core_panic(core, -1, "Invalid comparison type");
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

    const char* namePtr = &core->constPtr[offset];
    char* name = strdup(namePtr);
    engine_ffi_register(core->engineRef, name, id);
}

static inline void open_ffi(TypeV_Core* core){
    uint16_t ffi_id = 0;
    typev_memcpy_u64_ptr(&ffi_id, &core->codePtr[core->ip], 2);
    core->ip += 2;

    engine_ffi_open(core->engineRef, ffi_id);
}

static inline void ld_ffi(TypeV_Core* core){
    uint8_t dest = core->codePtr[core->ip++];
    ASSERT(dest < MAX_REG, "Invalid register index");

    uint16_t id = 0;
    typev_memcpy_u64_ptr(&id, &core->codePtr[core->ip], 2);
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

static inline void spill_alloc(TypeV_Core* core){
    uint16_t size = (uint16_t)typev_memcpy_u64(&core->codePtr[core->ip], 2);
    core->ip += 2;
    core_spill_alloc(core, size);
}

static inline void spill_reg(TypeV_Core* core){
    uint16_t slot = (uint16_t)typev_memcpy_u64(&core->codePtr[core->ip], 2);
    core->ip += 2;
    uint8_t reg = core->codePtr[core->ip++];
    core->funcState->spillSlots[slot] = core->regs[reg];
}


static inline void unspill_reg(TypeV_Core* core){
    uint8_t reg = core->codePtr[core->ip++];
    uint16_t slot = (uint16_t)typev_memcpy_u64(&core->codePtr[core->ip], 2);
    core->ip += 2;
    core->regs[reg] = core->funcState->spillSlots[slot];
}

#endif //TYPE_V_INSTRUCTIONS_H
