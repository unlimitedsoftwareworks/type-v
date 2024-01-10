//
// Created by praisethemoon on 21.11.23.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "instructions.h"
#include "../std/std.h"
#include "../core.h"
#include "../utils/utils.h"
#include "../utils/log.h"
#include "../vendor/libtable/table.h"
#include "../stack/stack.h"
#include "../engine.h"


#define CORE_ASSERT(condition, message) \
if(!(condition)) { \
    core_panic(core, 0, message); \
}

static inline uint8_t isValidByte(uint8_t byte) {
    switch(byte) {
        case 1:
        case 2:
        case 4:
        case 8:
            return 1;
        default:
            return 0;
    }
}

// for aligned data
static inline void typev_memcpy(void* dest, const void* src, size_t n) {
    // Assuming dest and src are properly aligned for their respective types
    switch (n) {
        case 1:
            *(uint8_t*)dest = *(const uint8_t*)src;
            break;
        case 2:
            *(uint16_t*)dest = *(const uint16_t*)src;
            break;
        case 3:
            *(uint16_t*)dest = *(const uint16_t*)src;
            *((uint8_t*)dest + 2) = *((const uint8_t*)src + 2);
            break;
        case 4:
            *(uint32_t*)dest = *(const uint32_t*)src;
            break;
        case 8:
            *(uint32_t*)dest = *(const uint32_t*)src;
            *(uint32_t*)((uint8_t*)dest + n - 4) = *(const uint32_t*)((const uint8_t*)src + n - 4);
            break;
        default:
            // For unsupported sizes, default to regular memcpy or handle as needed
            break;
    }
}

// for possibly unaligned data
static inline void typev_umemcpy(void* dest, const void* src, size_t n) {
    uint8_t *d = (uint8_t *) dest;
    const uint8_t *s = (const uint8_t *) src;

    while (n--) {
        *d++ = *s++;
    }
}

void mv_reg_reg(TypeV_Core* core){
    const uint8_t target = core->codePtr[core->ip++];
    const uint8_t source = core->codePtr[core->ip++];
    uint8_t byteSize = core->codePtr[core->ip++];
    CORE_ASSERT(isValidByte(byteSize), "Invalid byte size");

    typev_memcpy(&core->regs[target], &core->regs[source], byteSize);
}

void mv_reg_reg_ptr(TypeV_Core* core){
    const uint8_t target = core->codePtr[core->ip++];
    const uint8_t source = core->codePtr[core->ip++];

    typev_memcpy(&core->regs[target], &core->regs[source], PTR_SIZE);
}

void mv_reg_null(TypeV_Core* core){
    const uint8_t target = core->codePtr[core->ip++];
    core->regs[target].ptr = 0;
}

void mv_reg_i(TypeV_Core* core){
    const uint8_t target = core->codePtr[core->ip++];
    uint8_t immediate_size = core->codePtr[core->ip++];
    uint64_t immediate = 0;
    // data could be unaligned
    typev_umemcpy(&immediate, &core->codePtr[core->ip], immediate_size);
    core->ip += immediate_size;
    typev_umemcpy(&core->regs[target], &immediate, 8);
}


void mv_reg_const(TypeV_Core* core){
    const uint8_t target = core->codePtr[core->ip++];
    const uint8_t offset_length = core->codePtr[core->ip++];
    size_t constant_offset = 0;
    // data could be unaligned
    typev_umemcpy(&constant_offset, &core->codePtr[core->ip],  offset_length);

    uint8_t byteSize = core->codePtr[core->ip++];

    CORE_ASSERT(isValidByte(byteSize), "Invalid byte size");

    core->ip += offset_length;
    typev_umemcpy(&core->regs[target], &core->constPtr[constant_offset], byteSize);
}

/*
void mv_reg_const_8(TypeV_Core* core){
    const uint8_t target = core->codePtr[core->ip++];

    const uint8_t offset_length = core->codePtr[core->ip++];
    size_t constant_offset = 0;
    // data could be unaligned
    typev_umemcpy(&constant_offset, &core->codePtr[core->ip],  offset_length);
    core->ip += offset_length;
    typev_umemcpy(&core->regs[target], &core->constPtr[constant_offset], 1);
}

void mv_reg_const_16(TypeV_Core* core){
    const uint8_t target = core->codePtr[core->ip++];

    const uint8_t offset_length = core->codePtr[core->ip++];
    size_t constant_offset = 0;
    // data could be unaligned
    typev_umemcpy(&constant_offset, &core->codePtr[core->ip],  offset_length);
    core->ip += offset_length;
    typev_umemcpy(&core->regs[target], &core->constPtr[constant_offset], 2);
}

void mv_reg_const_32(TypeV_Core* core){
    const uint8_t target = core->codePtr[core->ip++];

    const uint8_t offset_length = core->codePtr[core->ip++];
    size_t constant_offset = 0;
    // data could be unaligned
    typev_umemcpy(&constant_offset, &core->codePtr[core->ip],  offset_length);
    core->ip += offset_length;
    typev_umemcpy(&core->regs[target], &core->constPtr[constant_offset], 4);
}

void mv_reg_const_64(TypeV_Core* core){
    const uint8_t target = core->codePtr[core->ip++];

    const uint8_t offset_length = core->codePtr[core->ip++];
    size_t constant_offset = 0;
    // data could be unaligned
    typev_umemcpy(&constant_offset, &core->codePtr[core->ip],  offset_length);
    core->ip += offset_length;
    typev_umemcpy(&core->regs[target], &core->constPtr[constant_offset], 8);
}
*/

void mv_reg_const_ptr(TypeV_Core* core){
    const uint8_t target = core->codePtr[core->ip++];
    
    const uint8_t offset_length = core->codePtr[core->ip++];
    size_t constant_offset = 0; 
    // data could be unaligned
    typev_umemcpy(&constant_offset, &core->codePtr[core->ip],  offset_length);
    core->ip += offset_length;
    typev_umemcpy(&core->regs[target], &core->constPtr[constant_offset], PTR_SIZE);
}

void mv_global_reg(TypeV_Core* core){
    const uint8_t offset_length = core->codePtr[core->ip++];
    size_t offset = 0;
    typev_umemcpy(&offset, &core->codePtr[core->ip],  offset_length);
    core->ip += offset_length;
    const uint8_t source = core->codePtr[core->ip++];
    uint8_t byteSize = core->codePtr[core->ip++];

    CORE_ASSERT(isValidByte(byteSize), "Invalid byte size");

    typev_memcpy(&core->globalPtr[offset], &core->regs[source], byteSize);
}

/*
void mv_global_reg_8(TypeV_Core* core){
    const uint8_t offset_length = core->codePtr[core->ip++];
    size_t offset = 0;
    typev_umemcpy(&offset, &core->codePtr[core->ip],  offset_length);
    core->ip += offset_length;
    const uint8_t source = core->codePtr[core->ip++];
    
    
    typev_memcpy(&core->globalPtr[offset], &core->regs[source], 1);
}

void mv_global_reg_16(TypeV_Core* core){
    const uint8_t offset_length = core->codePtr[core->ip++];
    size_t offset = 0;
    typev_umemcpy(&offset, &core->codePtr[core->ip],  offset_length);
    core->ip += offset_length;
    const uint8_t source = core->codePtr[core->ip++];
    
    typev_umemcpy(&core->globalPtr[offset], &core->regs[source], 2);
}


void mv_global_reg_32(TypeV_Core* core){
    const uint8_t offset_length = core->codePtr[core->ip++];
    size_t offset = 0;
    typev_umemcpy(&offset, &core->codePtr[core->ip],  offset_length);
    core->ip += offset_length;
    const uint8_t source = core->codePtr[core->ip++];

    typev_umemcpy(&core->globalPtr[offset], &core->regs[source], 4);
}

void mv_global_reg_64(TypeV_Core* core){
    const uint8_t offset_length = core->codePtr[core->ip++];
    size_t offset = 0;
    typev_umemcpy(&offset, &core->codePtr[core->ip],  offset_length);
    core->ip += offset_length;
    const uint8_t source = core->codePtr[core->ip++];


    typev_umemcpy(&core->globalPtr[offset], &core->regs[source], 8);
}
 */

void mv_global_reg_ptr(TypeV_Core* core){
    const uint8_t offset_length = core->codePtr[core->ip++];
    size_t offset = 0;
    typev_umemcpy(&offset, &core->codePtr[core->ip],  offset_length);
    core->ip += offset_length;
    const uint8_t source = core->codePtr[core->ip++];

    typev_umemcpy(&core->globalPtr[offset], &core->regs[source], PTR_SIZE);
}

void mv_reg_global(TypeV_Core* core){
    const uint8_t target = core->codePtr[core->ip++];
    const uint8_t offset_length = core->codePtr[core->ip++];
    size_t offset = 0;
    typev_umemcpy(&offset, &core->codePtr[core->ip],  offset_length);
    core->ip += offset_length;
    uint8_t byteSize = core->codePtr[core->ip++];
    CORE_ASSERT(isValidByte(byteSize), "Invalid byte size");

    typev_umemcpy(&core->regs[target], &core->globalPtr[offset], byteSize);
}
/*
void mv_reg_global_8(TypeV_Core* core){
    const uint8_t target = core->codePtr[core->ip++];
    const uint8_t offset_length = core->codePtr[core->ip++];
    size_t offset = 0;
    typev_umemcpy(&offset, &core->codePtr[core->ip],  offset_length);
    core->ip += offset_length;


    typev_umemcpy(&core->regs[target], &core->globalPtr[offset], 1);
}

void mv_reg_global_16(TypeV_Core* core){
    const uint8_t target = core->codePtr[core->ip++];
    const uint8_t offset_length = core->codePtr[core->ip++];
    size_t offset = 0;
    typev_umemcpy(&offset, &core->codePtr[core->ip],  offset_length);
    core->ip += offset_length;


    typev_umemcpy(&core->regs[target], &core->globalPtr[offset], 2);
}

void mv_reg_global_32(TypeV_Core* core){
    const uint8_t target = core->codePtr[core->ip++];
    const uint8_t offset_length = core->codePtr[core->ip++];
    size_t offset = 0;
    typev_umemcpy(&offset, &core->codePtr[core->ip],  offset_length);
    core->ip += offset_length;


    typev_umemcpy(&core->regs[target], &core->globalPtr[offset], 4);
}

void mv_reg_global_64(TypeV_Core* core){
    const uint8_t target = core->codePtr[core->ip++];
    const uint8_t offset_length = core->codePtr[core->ip++];
    size_t offset = 0;
    typev_umemcpy(&offset, &core->codePtr[core->ip],  offset_length);
    core->ip += offset_length;


    typev_umemcpy(&core->regs[target], &core->globalPtr[offset], 8);
}
*/

void mv_reg_global_ptr(TypeV_Core* core){
    const uint8_t target = core->codePtr[core->ip++];
    const uint8_t offset_length = core->codePtr[core->ip++];
    size_t offset = 0;
    typev_umemcpy(&offset, &core->codePtr[core->ip],  offset_length);
    core->ip += offset_length;


    typev_umemcpy(&core->regs[target], &core->globalPtr[offset], PTR_SIZE);
}

void s_alloc(TypeV_Core* core){
    const uint8_t dest_reg = core->codePtr[core->ip++];
    ASSERT(dest_reg < MAX_REG, "Invalid register index");
    const uint8_t fields_count = core->codePtr[core->ip++];
    uint16_t struct_size = 0;
    typev_umemcpy(&struct_size, &core->codePtr[core->ip],  2);
    core->ip += 2;

    // allocate memory for struct
    size_t mem = core_struct_alloc(core, fields_count, struct_size);
    // move the pointer to R16
    core->regs[dest_reg].ptr = mem;
}


void s_alloc_shadow(TypeV_Core* core){
    const uint8_t dest_reg = core->codePtr[core->ip++];
    ASSERT(dest_reg < MAX_REG, "Invalid register index");
    const uint8_t source_reg = core->codePtr[core->ip++];
    ASSERT(source_reg < MAX_REG, "Invalid register index");
    const uint8_t fields_count = core->codePtr[core->ip++];

    uintptr_t original_mem = core->regs[source_reg].ptr;

    // allocate memory for struct
    uintptr_t  mem = core_struct_alloc_shadow(core, fields_count, original_mem);
    // move the pointer to R16
    core->regs[dest_reg].ptr = mem;
}

void s_set_offset(TypeV_Core* core){
    const uint8_t src_reg = core->codePtr[core->ip++];
    ASSERT(src_reg < MAX_REG, "Invalid register index");
    const uint8_t field_index = core->codePtr[core->ip++];
    uint16_t offset = 0;
    typev_umemcpy(&offset, &core->codePtr[core->ip],  2);
    core->ip += 2;

    TypeV_Struct* struct_ptr = (TypeV_Struct*)core->regs[src_reg].ptr;
    struct_ptr->fieldOffsets[field_index] = offset;
}

void s_set_offset_shadow(TypeV_Core* core){
    const uint8_t src_reg = core->codePtr[core->ip++];
    ASSERT(src_reg < MAX_REG, "Invalid register index");

    const uint8_t field_source_index = core->codePtr[core->ip++];
    const uint8_t field_target_index = core->codePtr[core->ip++];

    TypeV_Struct* struct_ptr = (TypeV_Struct*)core->regs[src_reg].ptr;
    ASSERT(struct_ptr->originalStruct != NULL, "Cannot set offset of shadow struct without original struct");
    struct_ptr->fieldOffsets[field_target_index] = struct_ptr->originalStruct->fieldOffsets[field_source_index];
}


void s_loadf(TypeV_Core* core){
    const uint8_t target = core->codePtr[core->ip++];
    const uint8_t source = core->codePtr[core->ip++];
    const uint8_t field_index = core->codePtr[core->ip++];

    uint8_t byteSize = core->codePtr[core->ip++];
    CORE_ASSERT(isValidByte(byteSize), "Invalid byte size");

    TypeV_Struct* struct_ptr = (TypeV_Struct*)core->regs[source].ptr;
    typev_memcpy(&core->regs[target], struct_ptr->dataPointer+struct_ptr->fieldOffsets[field_index], byteSize);
}

/*
void s_loadf_8(TypeV_Core* core){
    const uint8_t target = core->codePtr[core->ip++];
    
    const uint8_t source = core->codePtr[core->ip++];
    
    const uint8_t field_index = core->codePtr[core->ip++];
    TypeV_Struct* struct_ptr = (TypeV_Struct*)core->regs[source].ptr;
    typev_memcpy(&core->regs[target], struct_ptr->dataPointer+struct_ptr->fieldOffsets[field_index], 1);
}

void s_loadf_16(TypeV_Core* core){
    const uint8_t target = core->codePtr[core->ip++];
    
    const uint8_t source = core->codePtr[core->ip++];
    
    const uint8_t field_index = core->codePtr[core->ip++];
    TypeV_Struct* struct_ptr = (TypeV_Struct*)core->regs[source].ptr;
    typev_memcpy(&core->regs[target], struct_ptr->dataPointer+struct_ptr->fieldOffsets[field_index], 2);
}

void s_loadf_32(TypeV_Core* core){
    const uint8_t target = core->codePtr[core->ip++];
    
    const uint8_t source = core->codePtr[core->ip++];
    
    const uint8_t field_index = core->codePtr[core->ip++];
    TypeV_Struct* struct_ptr = (TypeV_Struct*)core->regs[source].ptr;
    typev_memcpy(&core->regs[target], struct_ptr->dataPointer+struct_ptr->fieldOffsets[field_index], 4);
}

void s_loadf_64(TypeV_Core* core){
    const uint8_t target = core->codePtr[core->ip++];
    
    const uint8_t source = core->codePtr[core->ip++];
    
    const uint8_t field_index = core->codePtr[core->ip++];
    TypeV_Struct* struct_ptr = (TypeV_Struct*)core->regs[source].ptr;
    typev_memcpy(&core->regs[target], struct_ptr->dataPointer+struct_ptr->fieldOffsets[field_index], 8);
}
*/

void s_loadf_ptr(TypeV_Core* core){
    const uint8_t target = core->codePtr[core->ip++];
    
    const uint8_t source = core->codePtr[core->ip++];
    
    const uint8_t field_index = core->codePtr[core->ip++];
    TypeV_Struct* struct_ptr = (TypeV_Struct*)core->regs[source].ptr;
    typev_memcpy(&core->regs[target], struct_ptr->dataPointer+struct_ptr->fieldOffsets[field_index], PTR_SIZE);
}

void s_storef_const(TypeV_Core* core){
    const uint8_t dest_reg = core->codePtr[core->ip++];
    const uint8_t field_index = core->codePtr[core->ip++];
    size_t offset = 0;
    typev_umemcpy(&offset, &core->codePtr[core->ip], 8);
    core->ip += 8;
    uint8_t byteSize = core->codePtr[core->ip++];
    CORE_ASSERT(isValidByte(byteSize), "Invalid byte size");
    TypeV_Struct* struct_ptr = (TypeV_Struct*)core->regs[dest_reg].ptr;
    typev_umemcpy(struct_ptr->dataPointer+struct_ptr->fieldOffsets[field_index], &core->constPtr[offset], byteSize);
}

/*
void s_storef_const_8(TypeV_Core* core){
    const uint8_t dest_reg = core->codePtr[core->ip++];
    const uint8_t field_index = core->codePtr[core->ip++];
    size_t offset = 0;
    typev_umemcpy(&offset, &core->codePtr[core->ip], 8);
    core->ip += 8;
    TypeV_Struct* struct_ptr = (TypeV_Struct*)core->regs[dest_reg].ptr;
    typev_umemcpy(struct_ptr->dataPointer+struct_ptr->fieldOffsets[field_index], &core->constPtr[offset], 1);
}

void s_storef_const_16(TypeV_Core* core){
    const uint8_t dest_reg = core->codePtr[core->ip++];
    const uint8_t field_index = core->codePtr[core->ip++];
    size_t offset = 0;
    typev_umemcpy(&offset, &core->codePtr[core->ip], 8);
    core->ip += 8;
    TypeV_Struct* struct_ptr = (TypeV_Struct*)core->regs[dest_reg].ptr;
    typev_umemcpy(struct_ptr->dataPointer+struct_ptr->fieldOffsets[field_index], &core->constPtr[offset], 2);
}

void s_storef_const_32(TypeV_Core* core){
    const uint8_t dest_reg = core->codePtr[core->ip++];
    const uint8_t field_index = core->codePtr[core->ip++];
    size_t offset = 0;
    typev_umemcpy(&offset, &core->codePtr[core->ip], 8);
    core->ip += 8;
    TypeV_Struct* struct_ptr = (TypeV_Struct*)core->regs[dest_reg].ptr;
    typev_umemcpy(struct_ptr->dataPointer+struct_ptr->fieldOffsets[field_index], &core->constPtr[offset], 4);
}

void s_storef_const_64(TypeV_Core* core){
    const uint8_t dest_reg = core->codePtr[core->ip++];
    const uint8_t field_index = core->codePtr[core->ip++];
    size_t offset = 0;
    typev_umemcpy(&offset, &core->codePtr[core->ip], 8);
    core->ip += 8;
    TypeV_Struct* struct_ptr = (TypeV_Struct*)core->regs[dest_reg].ptr;
    typev_umemcpy(struct_ptr->dataPointer+struct_ptr->fieldOffsets[field_index], &core->constPtr[offset], 8);
}
*/

void s_storef_const_ptr(TypeV_Core* core){
    const uint8_t dest_reg = core->codePtr[core->ip++];
    const uint8_t field_index = core->codePtr[core->ip++];
    size_t offset = 0;
    typev_umemcpy(&offset, &core->codePtr[core->ip], 8);
    core->ip += 8;
    TypeV_Struct* struct_ptr = (TypeV_Struct*)core->regs[dest_reg].ptr;
    typev_umemcpy(struct_ptr->dataPointer+struct_ptr->fieldOffsets[field_index], &core->constPtr[offset], PTR_SIZE);
}

void s_storef_reg(TypeV_Core* core){
    const uint8_t dest_reg = core->codePtr[core->ip++];
    const uint8_t field_index = core->codePtr[core->ip++];
    ASSERT(field_index < MAX_REG, "Invalid register index");
    const uint8_t source = core->codePtr[core->ip++];
    uint8_t byteSize = core->codePtr[core->ip++];
    CORE_ASSERT(isValidByte(byteSize), "Invalid byte size");

    TypeV_Struct *struct_ptr = (TypeV_Struct *) core->regs[dest_reg].ptr;
    typev_memcpy(struct_ptr->dataPointer + struct_ptr->fieldOffsets[field_index], &core->regs[source], byteSize);
}

/*
void s_storef_reg_8(TypeV_Core* core){
    const uint8_t dest_reg = core->codePtr[core->ip++];
    const uint8_t field_index = core->codePtr[core->ip++];
    ASSERT(field_index < MAX_REG, "Invalid register index");
    const uint8_t source = core->codePtr[core->ip++];
    
    TypeV_Struct *struct_ptr = (TypeV_Struct *) core->regs[dest_reg].ptr;
    typev_memcpy(struct_ptr->dataPointer + struct_ptr->fieldOffsets[field_index], &core->regs[source], 1);
}

void s_storef_reg_16(TypeV_Core* core){
    const uint8_t dest_reg = core->codePtr[core->ip++];
    const uint8_t field_index = core->codePtr[core->ip++];
    ASSERT(field_index < MAX_REG, "Invalid register index");
    const uint8_t source = core->codePtr[core->ip++];
    
    TypeV_Struct *struct_ptr = (TypeV_Struct *) core->regs[dest_reg].ptr;
    typev_memcpy(struct_ptr->dataPointer + struct_ptr->fieldOffsets[field_index], &core->regs[source], 2);
}

void s_storef_reg_32(TypeV_Core* core){
    const uint8_t dest_reg = core->codePtr[core->ip++];
    const uint8_t field_index = core->codePtr[core->ip++];
    ASSERT(field_index < MAX_REG, "Invalid register index");
    const uint8_t source = core->codePtr[core->ip++];
    
    TypeV_Struct *struct_ptr = (TypeV_Struct *) core->regs[dest_reg].ptr;
    typev_memcpy(struct_ptr->dataPointer + struct_ptr->fieldOffsets[field_index], &core->regs[source], 4);
}

void s_storef_reg_64(TypeV_Core* core){
    const uint8_t dest_reg = core->codePtr[core->ip++];
    const uint8_t field_index = core->codePtr[core->ip++];
    ASSERT(field_index < MAX_REG, "Invalid register index");
    const uint8_t source = core->codePtr[core->ip++];
    
    TypeV_Struct *struct_ptr = (TypeV_Struct *) core->regs[dest_reg].ptr;
    typev_memcpy(struct_ptr->dataPointer + struct_ptr->fieldOffsets[field_index], &core->regs[source], 8);
}
*/

void s_storef_reg_ptr(TypeV_Core* core){
    const uint8_t dest_reg = core->codePtr[core->ip++];
    const uint8_t field_index = core->codePtr[core->ip++];
    ASSERT(field_index < MAX_REG, "Invalid register index");
    const uint8_t source = core->codePtr[core->ip++];
    
    TypeV_Struct *struct_ptr = (TypeV_Struct *) core->regs[dest_reg].ptr;
    typev_memcpy(struct_ptr->dataPointer + struct_ptr->fieldOffsets[field_index], &core->regs[source], 8);
}


void c_alloc(TypeV_Core* core){
    const uint8_t dest_reg = core->codePtr[core->ip++];
    ASSERT(dest_reg < MAX_REG, "Invalid register index");
    const uint8_t methods_count = core->codePtr[core->ip++];
    size_t fields_size = 0;
    typev_umemcpy(&fields_size, &core->codePtr[core->ip],  2);
    core->ip += 2;

    uint8_t classId_size = core->codePtr[core->ip++];
    uint64_t classId = 0;
    typev_umemcpy(&classId, &core->codePtr[core->ip],  classId_size);
    core->ip += classId_size;


    // allocate memory for class
    size_t mem = core_class_alloc(core, methods_count, fields_size, classId);
    // move the pointer to R17
    core->regs[dest_reg].ptr = mem;
}

void c_storem(TypeV_Core* core){
    const uint8_t dest_reg = core->codePtr[core->ip++];
    ASSERT(dest_reg < MAX_REG, "Invalid register index");
    const uint8_t method_index = core->codePtr[core->ip++];
    size_t methd_address = 0; /* we do not increment methd_address here*/
    typev_umemcpy(&methd_address, &core->codePtr[core->ip], 8);
    core->ip += 8;

    TypeV_Class* c = (TypeV_Class*)core->regs[dest_reg].ptr;
    LOG_INFO("Storing method %d at methd_address %d in class %p", method_index, methd_address, (void*)c);
    c->methods[method_index] = methd_address;
}

void c_loadm(TypeV_Core* core){
    const uint8_t target = core->codePtr[core->ip++];
    const uint8_t class_reg = core->codePtr[core->ip++];
    const uint8_t method_index = core->codePtr[core->ip++];

    TypeV_Class* c = (TypeV_Class*)core->regs[class_reg].ptr;

    LOG_INFO("Loading method %d from class %p", method_index, (void*)c);

    size_t offset = c->methods[method_index];
    core->regs[target].ptr = offset;
}

void c_storef_reg(TypeV_Core* core){
    const uint8_t class_reg = core->codePtr[core->ip++];
    size_t field_offset = 0;
    typev_umemcpy(&field_offset, &core->codePtr[core->ip], 2);
    core->ip += 2;
    const uint8_t source = core->codePtr[core->ip++];
    uint8_t byteSize = core->codePtr[core->ip++];
    CORE_ASSERT(isValidByte(byteSize), "Invalid byte size");

    TypeV_Class* c = (TypeV_Class*)core->regs[class_reg].ptr;
    typev_memcpy(c->data+field_offset, &core->regs[source].ptr, byteSize);
}

/*
void c_storef_reg_8(TypeV_Core* core){
    const uint8_t class_reg = core->codePtr[core->ip++];
    size_t field_offset = 0;
    typev_umemcpy(&field_offset, &core->codePtr[core->ip], 2);
    core->ip += 2;
    const uint8_t source = core->codePtr[core->ip++];

    TypeV_Class* c = (TypeV_Class*)core->regs[class_reg].ptr;
    typev_memcpy(c->data+field_offset, &core->regs[source].ptr, 1);
}

void c_storef_reg_16(TypeV_Core* core){
    const uint8_t class_reg = core->codePtr[core->ip++];
    size_t field_offset = 0;
    typev_umemcpy(&field_offset, &core->codePtr[core->ip], 2);
    core->ip += 2;
    const uint8_t source = core->codePtr[core->ip++];

    TypeV_Class* c = (TypeV_Class*)core->regs[class_reg].ptr;
    typev_memcpy(c->data+field_offset, &core->regs[source].ptr, 2);
}

void c_storef_reg_32(TypeV_Core* core){
    const uint8_t class_reg = core->codePtr[core->ip++];
    size_t field_offset = 0;
    typev_umemcpy(&field_offset, &core->codePtr[core->ip], 2);
    core->ip += 2;
    const uint8_t source = core->codePtr[core->ip++];

    TypeV_Class* c = (TypeV_Class*)core->regs[class_reg].ptr;
    typev_memcpy(c->data+field_offset, &core->regs[source].ptr, 4);
}

void c_storef_reg_64(TypeV_Core* core){
    const uint8_t class_reg = core->codePtr[core->ip++];
    size_t field_offset = 0;
    typev_umemcpy(&field_offset, &core->codePtr[core->ip], 2);
    core->ip += 2;
    const uint8_t source = core->codePtr[core->ip++];

    TypeV_Class* c = (TypeV_Class*)core->regs[class_reg].ptr;
    typev_memcpy(c->data+field_offset, &core->regs[source].ptr, 8);
}
*/

void c_storef_reg_ptr(TypeV_Core* core){
    const uint8_t class_reg = core->codePtr[core->ip++];
    ASSERT(class_reg < MAX_REG, "Invalid register index");
    size_t field_offset = 0;
    typev_umemcpy(&field_offset, &core->codePtr[core->ip], 2);
    core->ip += 2;
    const uint8_t source = core->codePtr[core->ip++];
    
    TypeV_Class* c = (TypeV_Class*)core->regs[class_reg].ptr;
    typev_memcpy(c->data+field_offset, &core->regs[source].ptr, PTR_SIZE);
}

void c_storef_const(TypeV_Core* core) {
    const uint8_t class_reg = core->codePtr[core->ip++];
    ASSERT(class_reg < MAX_REG, "Invalid register index");
    size_t field_offset = 0;
    typev_umemcpy(&field_offset, &core->codePtr[core->ip], 2);
    core->ip += 2;
    size_t offset = 0;
    typev_umemcpy(&offset, &core->codePtr[core->ip], 8);
    core->ip += 8;
    uint8_t byteSize = core->codePtr[core->ip++];
    CORE_ASSERT(isValidByte(byteSize), "Invalid byte size");

    TypeV_Class *c = (TypeV_Class *) core->regs[class_reg].ptr;
    typev_umemcpy(c->data + field_offset, &core->constPtr[offset], byteSize);
}

void c_storef_const_ptr(TypeV_Core* core){
    const uint8_t class_reg = core->codePtr[core->ip++];
    ASSERT(class_reg < MAX_REG, "Invalid register index");
    size_t field_offset = 0;
    typev_umemcpy(&field_offset, &core->codePtr[core->ip], 2);
    core->ip += 2;
    size_t offset = 0;
    typev_umemcpy(&offset, &core->codePtr[core->ip], 8);
    core->ip += 8;

    TypeV_Class* c = (TypeV_Class*)core->regs[class_reg].ptr;
    typev_umemcpy(c->data+field_offset, &core->constPtr[offset], PTR_SIZE);
}

void c_loadf(TypeV_Core* core){
    const uint8_t target = core->codePtr[core->ip++];
    const uint8_t class_reg = core->codePtr[core->ip++];
    size_t offset = 0;
    typev_umemcpy(&offset, &core->codePtr[core->ip], 2);
    core->ip += 2;
    uint8_t byteSize = core->codePtr[core->ip++];
    CORE_ASSERT(isValidByte(byteSize), "Invalid byte size");
    TypeV_Class* c = (TypeV_Class*)core->regs[class_reg].ptr;
    typev_memcpy(&core->regs[target], c->data+offset, byteSize);
}

/*
void c_loadf_8(TypeV_Core* core){
    const uint8_t target = core->codePtr[core->ip++];
    const uint8_t class_reg = core->codePtr[core->ip++];
    size_t offset = 0;
    typev_umemcpy(&offset, &core->codePtr[core->ip], 2);
    core->ip += 2;
    TypeV_Class* c = (TypeV_Class*)core->regs[class_reg].ptr;
    typev_memcpy(&core->regs[target], c->data+offset, 1);
}

void c_loadf_16(TypeV_Core* core){
    const uint8_t target = core->codePtr[core->ip++];
    const uint8_t class_reg = core->codePtr[core->ip++];
    size_t offset = 0;
    typev_umemcpy(&offset, &core->codePtr[core->ip], 2);
    core->ip += 2;
    TypeV_Class* c = (TypeV_Class*)core->regs[class_reg].ptr;
    typev_memcpy(&core->regs[target], c->data+offset, 2);
}

void c_loadf_32(TypeV_Core* core){
    const uint8_t target = core->codePtr[core->ip++];
    const uint8_t class_reg = core->codePtr[core->ip++];
    size_t offset = 0;
    typev_umemcpy(&offset, &core->codePtr[core->ip], 2);
    core->ip += 2;
    TypeV_Class* c = (TypeV_Class*)core->regs[class_reg].ptr;
    typev_memcpy(&core->regs[target], c->data+offset, 4);
}

void c_loadf_64(TypeV_Core* core){
    const uint8_t target = core->codePtr[core->ip++];
    const uint8_t class_reg = core->codePtr[core->ip++];
    size_t offset = 0;
    typev_umemcpy(&offset, &core->codePtr[core->ip], 2);
    core->ip += 2;
    TypeV_Class* c = (TypeV_Class*)core->regs[class_reg].ptr;
    typev_memcpy(&core->regs[target], c->data+offset, 8);
}
 */

void c_loadf_ptr(TypeV_Core* core){
    const uint8_t target = core->codePtr[core->ip++];
    const uint8_t class_reg = core->codePtr[core->ip++];
    size_t offset = 0;
    typev_umemcpy(&offset, &core->codePtr[core->ip], 2);
    core->ip += 2;
    TypeV_Class* c = (TypeV_Class*)core->regs[class_reg].ptr;
    typev_memcpy(&core->regs[target], c->data+offset, PTR_SIZE);
}

void i_alloc(TypeV_Core* core){
    const uint8_t dest_reg = core->codePtr[core->ip++];
    const uint8_t fields_count = core->codePtr[core->ip++];
    const uint8_t class_reg = core->codePtr[core->ip++];

    TypeV_Class* c = (TypeV_Class*)core->regs[class_reg].ptr;
    // allocate memory for struct
    size_t mem = core_interface_alloc(core, fields_count, c);
    // move the pointer to R18
    core->regs[dest_reg].ptr = mem;
}

void i_alloc_i(TypeV_Core* core){
    const uint8_t dest_reg = core->codePtr[core->ip++];
    const uint8_t methods_count = core->codePtr[core->ip++];
    const uint8_t interface_reg = core->codePtr[core->ip++];

    TypeV_Interface* i = (TypeV_Interface*)core->regs[interface_reg].ptr;
    size_t interface_new = core_interface_alloc_i(core, methods_count, i);
    core->regs[dest_reg].ptr = interface_new;
}

void i_set_offset(TypeV_Core* core){
    const uint8_t dest_reg = core->codePtr[core->ip++];
    const uint8_t field_index = core->codePtr[core->ip++];
    size_t offset = 0; 
    typev_memcpy(&offset, &core->codePtr[core->ip], 2);
    core->ip += 2;

    TypeV_Interface* i = (TypeV_Interface*)core->regs[dest_reg].ptr;
    i->methodsOffset[field_index] = offset;
}

void i_set_offset_i(TypeV_Core* core) {
    const uint8_t dest = core->codePtr[core->ip++];
    const uint8_t field_source = core->codePtr[core->ip++];
    const uint8_t field_target = core->codePtr[core->ip++];
    const uint8_t interface_reg = core->codePtr[core->ip++];

    TypeV_Interface* i = (TypeV_Interface*)core->regs[dest].ptr;
    TypeV_Interface* v = (TypeV_Interface*)core->regs[interface_reg].ptr;

    uint16_t z = i->methodsOffset[field_target];
    uint16_t y = v->methodsOffset[field_source];

    i->methodsOffset[field_target] = v->methodsOffset[field_source];
}


void i_set_offset_m(TypeV_Core* core){
    uint64_t lookUpMethodId = 0;
    typev_memcpy(&lookUpMethodId, &core->codePtr[core->ip],  8);
    core->ip += 8;

    uint16_t field_number = 0;
    typev_memcpy(&field_number, &core->codePtr[core->ip],  2);
    core->ip += 2;

    uint64_t jumpFailureAddress = 0;
    typev_memcpy(&jumpFailureAddress, &core->codePtr[core->ip],  8);
    core->ip += 8;

    TypeV_Interface* interface = (TypeV_Interface*)core->regs[16].ptr;
    TypeV_Class * class_ = interface->classPtr;

    uint8_t found = 0;
    for(size_t i = 0; i < class_->num_methods; i++) {
        uint64_t methodId = 0;
        typev_umemcpy(&methodId, &core->codePtr[class_->methods[i]],  8);
        if(lookUpMethodId == methodId) {
            found = 1;
            uint16_t offset = i;
            typev_memcpy(&interface->methodsOffset[field_number], &offset, 2);
            break;
        }
    }

    if(!found){
        core->ip = jumpFailureAddress;
    }
}

void i_loadm(TypeV_Core* core){
    const uint8_t target = core->codePtr[core->ip++];
    const uint8_t interface_reg = core->codePtr[core->ip++];
    const uint8_t method_index = core->codePtr[core->ip++];

    TypeV_Interface* i = (TypeV_Interface*)core->regs[interface_reg].ptr;
    size_t offset = i->methodsOffset[method_index];

    core->regs[target].ptr = i->classPtr->methods[offset];
}

void i_is_c(TypeV_Core* core){
    const uint8_t target = core->codePtr[core->ip++];
    const uint8_t interface_reg = core->codePtr[core->ip++];

    uint64_t classId = 0;
    typev_umemcpy(&classId, &core->codePtr[core->ip],  8);
    core->ip += 8;

    TypeV_Interface* interface = (TypeV_Interface*)core->regs[interface_reg].ptr;
    TypeV_Class * class_ = interface->classPtr;

    core->regs[target].u8 = class_->uid == classId;
}

void i_is_i(TypeV_Core* core){
    uint64_t lookUpMethodId = 0;
    typev_umemcpy(&lookUpMethodId, &core->codePtr[core->ip],  8);
    core->ip += 8;

    const uint8_t interface_reg = core->codePtr[core->ip++];

    size_t offset = 0;
    typev_umemcpy(&offset, &core->codePtr[core->ip], 8);
    core->ip += 8;

    TypeV_Interface* interface = (TypeV_Interface*)core->regs[interface_reg].ptr;
    TypeV_Class * class_ = interface->classPtr;

    uint8_t found = 0;
    for(size_t i = 0; i < class_->num_methods; i++) {
        uint64_t methodId = 0;
        typev_umemcpy(&methodId, &core->codePtr[class_->methods[i]]-8,  8);
        if(lookUpMethodId == methodId) {
            found = 1;
            break;
        }
    }

    if(!found){
        core->ip = offset;
    }
}

void i_get_c(TypeV_Core* core) {
    const uint8_t dest_reg = core->codePtr[core->ip++];
    const uint8_t src_reg = core->codePtr[core->ip++];

    TypeV_Interface* interface = (TypeV_Interface*)core->regs[src_reg].ptr;
    core->regs[dest_reg].ptr = (size_t)interface->classPtr;
}

void a_alloc(TypeV_Core* core){
    const uint8_t dest = core->codePtr[core->ip++];

    size_t num_elements = 0;
    typev_umemcpy(&num_elements, &core->codePtr[core->ip], 8);
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

void a_extend(TypeV_Core* core){
    const uint8_t target_array = core->codePtr[core->ip++];
    const uint8_t size_reg = core->codePtr[core->ip++];
    uint64_t num_elements = core->regs[size_reg].u64;

    size_t mem = core_array_extend(core, core->regs[target_array].ptr, num_elements);
    core->regs[target_array].ptr = mem;
}

void a_len(TypeV_Core* core){
    const uint8_t target = core->codePtr[core->ip++];
    
    const uint8_t array_reg = core->codePtr[core->ip++];
    TypeV_Array* array = (TypeV_Array*)core->regs[array_reg].ptr;
    core->regs[target].u64 = array->length;
}

void a_storef_reg(TypeV_Core* core){
    const uint8_t array_reg = core->codePtr[core->ip++];
    const uint8_t index = core->codePtr[core->ip++];
    const uint8_t source = core->codePtr[core->ip++];
    uint8_t byteSize = core->codePtr[core->ip++];
    CORE_ASSERT(isValidByte(byteSize), "Invalid byte size");

    TypeV_Array* array = (TypeV_Array*)core->regs[array_reg].ptr;
    ASSERT(core->regs[index].u64 < array->length, "Index out of bounds");
    typev_memcpy(array->data+(core->regs[index].u64*array->elementSize), &core->regs[source], byteSize);
}

/*
void a_storef_reg_8(TypeV_Core* core){
    const uint8_t array_reg = core->codePtr[core->ip++];
    const uint8_t index = core->codePtr[core->ip++];
    const uint8_t source = core->codePtr[core->ip++];
    
    TypeV_Array* array = (TypeV_Array*)core->regs[array_reg].ptr;
    ASSERT(core->regs[index].u64 < array->length, "Index out of bounds");
    typev_memcpy(array->data+(core->regs[index].u64*array->elementSize), &core->regs[source], 1);
}

void a_storef_reg_16(TypeV_Core* core){
    const uint8_t array_reg = core->codePtr[core->ip++];
    ASSERT(array_reg < MAX_REG, "Invalid register index");
    const uint8_t index = core->codePtr[core->ip++];
    ASSERT(index < MAX_REG, "Invalid register index");
    const uint8_t source = core->codePtr[core->ip++];
    
    TypeV_Array* array = (TypeV_Array*)core->regs[array_reg].ptr;
    ASSERT(core->regs[index].u64 < array->length, "Index out of bounds");
    typev_memcpy(array->data+(core->regs[index].u64*array->elementSize), &core->regs[source], 2);
}

void a_storef_reg_32(TypeV_Core* core){
    const uint8_t array_reg = core->codePtr[core->ip++];
    const uint8_t index = core->codePtr[core->ip++];
    const uint8_t source = core->codePtr[core->ip++];
    
    TypeV_Array* array = (TypeV_Array*)core->regs[array_reg].ptr;
    ASSERT(core->regs[index].u64 < array->length, "Index out of bounds");
    typev_memcpy(array->data+(core->regs[index].u64*array->elementSize), &core->regs[source], 4);
}

void a_storef_reg_64(TypeV_Core* core){
    const uint8_t array_reg = core->codePtr[core->ip++];
    const uint8_t index = core->codePtr[core->ip++];
    const uint8_t source = core->codePtr[core->ip++];
    
    TypeV_Array* array = (TypeV_Array*)core->regs[array_reg].ptr;
    typev_memcpy(array->data+(core->regs[index].u64*array->elementSize), &core->regs[source], 8);
}
 */

void a_storef_reg_ptr(TypeV_Core* core){
    const uint8_t array_reg = core->codePtr[core->ip++];
    const uint8_t index = core->codePtr[core->ip++];
    const uint8_t source = core->codePtr[core->ip++];
    
    TypeV_Array* array = (TypeV_Array*)core->regs[array_reg].ptr;

    if(core->regs[index].u64 >= array->length) {
        core_panic(core, 1, "Index out of bounds %d >= %d", core->regs[index].u64, array->length);
    }
    typev_memcpy(array->data+(core->regs[index].u64*array->elementSize), &core->regs[source], PTR_SIZE);
}

void a_storef_const(TypeV_Core* core){
    const uint8_t array_reg = core->codePtr[core->ip++];
    uint8_t indexReg = core->codePtr[core->ip++];
    size_t offset = 0;
    typev_umemcpy(&offset, &core->codePtr[core->ip], 8);
    core->ip += 8;

    uint8_t byteSize = core->codePtr[core->ip++];
    CORE_ASSERT(isValidByte(byteSize), "Invalid byte size");

    TypeV_Array* array = (TypeV_Array*)core->regs[array_reg].ptr;
    ASSERT(core->regs[indexReg].u64 < array->length, "Index out of bounds");
    typev_umemcpy(array->data+(core->regs[indexReg].u64*array->elementSize), &core->constPtr[offset], byteSize);
}

/*
void a_storef_const_8(TypeV_Core* core){
    const uint8_t array_reg = core->codePtr[core->ip++];
    uint8_t indexReg = core->codePtr[core->ip++];
    size_t offset = 0;
    typev_umemcpy(&offset, &core->codePtr[core->ip], 8);
    core->ip += 8;

    TypeV_Array* array = (TypeV_Array*)core->regs[array_reg].ptr;
    ASSERT(core->regs[indexReg].u64 < array->length, "Index out of bounds");
    typev_umemcpy(array->data+(core->regs[indexReg].u64*array->elementSize), &core->constPtr[offset], 1);
}

void a_storef_const_16(TypeV_Core* core){
    const uint8_t array_reg = core->codePtr[core->ip++];
    uint8_t indexReg = core->codePtr[core->ip++];
    size_t offset = 0;
    typev_umemcpy(&offset, &core->codePtr[core->ip], 8);
    core->ip += 8;
    TypeV_Array* array = (TypeV_Array*)core->regs[array_reg].ptr;
    ASSERT(core->regs[indexReg].u64 < array->length, "Index out of bounds");
    typev_umemcpy(array->data+(core->regs[indexReg].u64*array->elementSize), &core->constPtr[offset], 2);
}

void a_storef_const_32(TypeV_Core* core){
    const uint8_t array_reg = core->codePtr[core->ip++];
    uint8_t indexReg = core->codePtr[core->ip++];
    size_t offset = 0;
    typev_umemcpy(&offset, &core->codePtr[core->ip], 8);
    core->ip += 8;
    TypeV_Array* array = (TypeV_Array*)core->regs[array_reg].ptr;
    ASSERT(core->regs[indexReg].u64 < array->length, "Index out of bounds");
    typev_umemcpy(array->data+(core->regs[indexReg].u64*array->elementSize), &core->constPtr[offset], 4);
}

void a_storef_const_64(TypeV_Core* core){
    const uint8_t array_reg = core->codePtr[core->ip++];
    uint8_t indexReg = core->codePtr[core->ip++];
    size_t offset = 0;
    typev_umemcpy(&offset, &core->codePtr[core->ip], 8);
    core->ip += 8;
    TypeV_Array* array = (TypeV_Array*)core->regs[array_reg].ptr;
    ASSERT(core->regs[indexReg].u64 < array->length, "Index out of bounds");
    typev_umemcpy(array->data+(core->regs[indexReg].u64*array->elementSize), &core->constPtr[offset], 8);
}
*/

void a_storef_const_ptr(TypeV_Core* core){
    const uint8_t array_reg = core->codePtr[core->ip++];
    uint8_t indexReg = core->codePtr[core->ip++];
    size_t offset = 0;
    typev_umemcpy(&offset, &core->codePtr[core->ip], 8);
    core->ip += 8;
    TypeV_Array* array = (TypeV_Array*)core->regs[array_reg].ptr;
    ASSERT(core->regs[indexReg].u64 < array->length, "Index out of bounds");
    typev_umemcpy(array->data+(core->regs[indexReg].u64*array->elementSize), &core->constPtr[offset], PTR_SIZE);
}

void a_loadf(TypeV_Core* core) {
    const uint8_t target = core->codePtr[core->ip++];
    const uint8_t index = core->codePtr[core->ip++];
    const uint8_t array_reg = core->codePtr[core->ip++];

    ASSERT(array_reg <= 8, "Invalid byte size");
    TypeV_Array* array = (TypeV_Array*)core->regs[array_reg].ptr;

    uint64_t idx = core->regs[index].u64;

    if(idx >= array->length) {
        core_panic(core, 1, "Index out of bounds %d >= %d", idx, array->length);
    }


    uint8_t byteSize = core->codePtr[core->ip++];
    CORE_ASSERT(isValidByte(byteSize), "Invalid byte size");

    typev_memcpy(&core->regs[target], array->data + (idx * array->elementSize), byteSize);
}

/*
void a_loadf_8(TypeV_Core* core) {
    const uint8_t target = core->codePtr[core->ip++];
    const uint8_t index = core->codePtr[core->ip++];
    const uint8_t array_reg = core->codePtr[core->ip++];

    ASSERT(array_reg <= 8, "Invalid byte size");
    TypeV_Array* array = (TypeV_Array*)core->regs[array_reg].ptr;

    uint64_t idx = core->regs[index].u64;

    if(idx >= array->length) {
        core_panic(core, 1, "Index out of bounds %d >= %d", idx, array->length);
    }

    typev_memcpy(&core->regs[target], array->data + (idx * array->elementSize), 1);
}

void a_loadf_16(TypeV_Core* core) {
    const uint8_t target = core->codePtr[core->ip++];
    const uint8_t index = core->codePtr[core->ip++];
    const uint8_t array_reg = core->codePtr[core->ip++];
    
    ASSERT(array_reg <= 8, "Invalid byte size");
    TypeV_Array* array = (TypeV_Array*)core->regs[array_reg].ptr;
    ASSERT(core->regs[index].u64 < array->length, "Index out of bounds");
    typev_memcpy(&core->regs[target], array->data + (core->regs[index].u64 * array->elementSize), 2);
}

void a_loadf_32(TypeV_Core* core) {
    const uint8_t target = core->codePtr[core->ip++];
    const uint8_t index = core->codePtr[core->ip++];
    const uint8_t array_reg = core->codePtr[core->ip++];
    
    TypeV_Array* array = (TypeV_Array*)core->regs[array_reg].ptr;
    ASSERT(core->regs[index].u64 < array->length, "Index out of bounds");
    typev_memcpy(&core->regs[target], array->data + (core->regs[index].u64 * array->elementSize), 4);
}

void a_loadf_64(TypeV_Core* core) {
    const uint8_t target = core->codePtr[core->ip++];
    const uint8_t index = core->codePtr[core->ip++];
    const uint8_t array_reg = core->codePtr[core->ip++];
    
    TypeV_Array* array = (TypeV_Array*)core->regs[array_reg].ptr;
    ASSERT(core->regs[index].u64 < array->length, "Index out of bounds");
    typev_memcpy(&core->regs[target], array->data + (core->regs[index].u64 * array->elementSize), 8);
}
*/

void a_loadf_ptr(TypeV_Core* core) {
    const uint8_t target = core->codePtr[core->ip++];
    const uint8_t index = core->codePtr[core->ip++];
    const uint8_t array_reg = core->codePtr[core->ip++];
    
    TypeV_Array* array = (TypeV_Array*)core->regs[array_reg].ptr;

    uint64_t idx = core->regs[index].u64;
    if(idx >= array->length) {
        core_panic(core, 1, "Index out of bounds %d <= %d", idx, array->length);
        return;
    }

    typev_memcpy(&core->regs[target], array->data + (core->regs[index].u64 * array->elementSize), PTR_SIZE);
}


// I: 915
void push(TypeV_Core* core){
    const uint8_t source = core->codePtr[core->ip++];
    uint8_t bytesize = core->codePtr[core->ip++];
    if(bytesize == 0) bytesize = PTR_SIZE;

    
    ASSERT(bytesize <= 8, "Invalid byte size");

    switch(bytesize){
        case 0:
            stack_push_ptr(core, core->regs[source].ptr);
            break;
        case 1:
            stack_push_8(core, core->regs[source].u8);
            break;
        case 2:
            stack_push_16(core, core->regs[source].u16);
            break;
        case 4:
            stack_push_32(core, core->regs[source].u32);
            break;
        case 8:
            stack_push_64(core, core->regs[source].ptr);
            break;
        default:
            LOG_ERROR("Invalid byte size %d", bytesize);
            exit(-1);
    }
}

void push_const(TypeV_Core* core){
    const uint8_t offset_length = core->codePtr[core->ip++];
    size_t offset = 0; 
    typev_umemcpy(&offset, &core->codePtr[core->ip],  offset_length);
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

void pop(TypeV_Core* core){
    const uint8_t target = core->codePtr[core->ip++];
    uint8_t bytesize = core->codePtr[core->ip++];
    if(bytesize == 0) bytesize = PTR_SIZE;

    
    ASSERT(bytesize <= 8, "Invalid byte size");

    switch(bytesize){
        case 0:
            stack_pop_ptr(core, &core->regs[target].ptr);
            break;
        case 1:
            stack_pop_8(core, &core->regs[target].u8);
            break;
        case 2:
            stack_pop_16(core, &core->regs[target].u16);
            break;
        case 4:
            stack_pop_32(core, &core->regs[target].u32);
            break;
        case 8:
            stack_pop_64(core, &core->regs[target].u64);
            break;
        default:
            LOG_ERROR("Invalid byte size %d", bytesize);
            exit(-1);
    }
}

void fn_alloc(TypeV_Core* core){
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

void fn_set_reg(TypeV_Core* core){
    const uint8_t dest_reg = core->codePtr[core->ip++];
    const uint8_t source_reg = core->codePtr[core->ip++];

    uint8_t byteSize = core->codePtr[core->ip++];
    CORE_ASSERT(isValidByte(byteSize), "Invalid byte size");

    typev_memcpy(&core->funcState->next->regs[dest_reg], &core->regs[source_reg], byteSize);
}

void fn_set_reg_ptr(TypeV_Core* core){
    const uint8_t dest_reg = core->codePtr[core->ip++];
    const uint8_t source_reg = core->codePtr[core->ip++];

    core->funcState->next->regs[dest_reg] = core->regs[source_reg];
}

void fn_call(TypeV_Core* core){
    // get the register
    const uint8_t target = core->codePtr[core->ip++];
    const size_t adr = core->regs[target].ptr;

    // back up IP in funcState
    core->funcState->ip = core->ip;
    core->ip = adr;
    core->funcState = core->funcState->next;
}

void fn_calli(TypeV_Core* core){
    // same as fn_call but takes immediate value
    const uint8_t offset_length = core->codePtr[core->ip++];
    size_t offset = 0; 
    typev_umemcpy(&offset, &core->codePtr[core->ip],  offset_length);
    core->ip += offset_length;


    // jump to the address
    core->funcState->ip = core->ip;
    core->ip = offset;
    core->funcState = core->funcState->next;
}

void fn_ret(TypeV_Core* core){
    core->ip = core->funcState->prev->ip;
    core->funcState = core->funcState->prev;
}

void fn_get_ret_reg(TypeV_Core* core){
    const uint8_t dest_reg = core->codePtr[core->ip++];
    const uint8_t source_reg = core->codePtr[core->ip++];
    const uint8_t byteSize = core->codePtr[core->ip++];

    CORE_ASSERT(isValidByte(byteSize), "Invalid byte size");

    // we grab the register from the .next into the current one
    core->regs[dest_reg] = core->funcState->next->regs[source_reg];
}

void fn_get_ret_reg_ptr(TypeV_Core* core){
    const uint8_t dest_reg = core->codePtr[core->ip++];
    const uint8_t source_reg = core->codePtr[core->ip++];

    // we grab the register from the .next into the current one
    core->regs[dest_reg].ptr = core->funcState->next->regs[source_reg].ptr;
}

#define OP_CAST(d1, d2, type) \
void cast_##d1##_##d2(TypeV_Core* core){ \
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

void upcast_i(TypeV_Core* core) {
    uint8_t reg = core->codePtr[core->ip++];
    uint8_t from = core->codePtr[core->ip++];
    uint8_t to = core->codePtr[core->ip++];

    ASSERT(reg < MAX_REG, "Invalid register index");
    ASSERT(from <= 8 && to <= 8 && from < to, "Invalid byte sizes for upcasting");

    // Extract the value from the register
    int64_t value = 0;
    typev_memcpy(&value, &core->regs[reg], from);

    // Perform sign extension without branching
    if (from < 8) {
        int64_t signExtMask = (value & (1LL << (from * 8 - 1))) ? (-1LL << (from * 8)) : 0;
        value |= signExtMask;
    }

    // Store the result back in the register
    typev_memcpy(&core->regs[reg], &value, to);
}

void upcast_u(TypeV_Core* core) {
    uint8_t reg = core->codePtr[core->ip++];
    uint8_t from = core->codePtr[core->ip++];
    uint8_t to = core->codePtr[core->ip++];

    ASSERT(reg < MAX_REG, "Invalid register index");
    ASSERT(from <= 8 && to <= 8 && from < to, "Invalid byte sizes for upcasting");

    // Extract the value from the register
    uint64_t value = 0;
    typev_memcpy(&value, &core->regs[reg], from);

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
    //typev_memcpy(&core->regs[reg], &value, to);
}

void upcast_f(TypeV_Core* core) {
    uint8_t reg = core->codePtr[core->ip++];
    uint8_t from = core->codePtr[core->ip++];
    uint8_t to = core->codePtr[core->ip++];

    ASSERT(reg < MAX_REG, "Invalid register index");
    ASSERT((from == 4 && to == 8), "Invalid byte sizes for floating-point upcasting");

    // Extract the float value from the register
    float floatValue = 0.0f;
    typev_memcpy(&floatValue, &core->regs[reg], from);

    // Convert to double
    double doubleValue = (double)floatValue;

    // Store the result back in the register
    typev_memcpy(&core->regs[reg], &doubleValue, to);
}


void dcast_i(TypeV_Core* core) {
    uint8_t reg = core->codePtr[core->ip++];
    uint8_t from = core->codePtr[core->ip++];
    uint8_t to = core->codePtr[core->ip++];

    ASSERT(reg < MAX_REG, "Invalid register index");
    ASSERT(from <= 8 && to <= 8 && from > to, "Invalid byte sizes for downcasting");

    // We simply need to copy the lower 'to' bytes from the register
    int64_t value = 0;
    typev_memcpy(&value, &core->regs[reg], to);

    // Store the result back in the register (overwriting only 'to' bytes)
    typev_memcpy(&core->regs[reg], &value, to);
}

void dcast_u(TypeV_Core* core) {
    uint8_t reg = core->codePtr[core->ip++];
    uint8_t from = core->codePtr[core->ip++];
    uint8_t to = core->codePtr[core->ip++];

    ASSERT(reg < MAX_REG, "Invalid register index");
    ASSERT(from <= 8 && to <= 8 && from > to, "Invalid byte sizes for downcasting");

    // Extract the value from the register
    uint64_t value = 0;
    typev_memcpy(&value, &core->regs[reg], from);

    // Truncate the value to the smaller size
    uint64_t truncatedValue = 0;
    typev_memcpy(&truncatedValue, &value, to);

    // Store the truncated value back in the register
    typev_memcpy(&core->regs[reg], &truncatedValue, to);
}

void dcast_f(TypeV_Core* core) {
    uint8_t reg = core->codePtr[core->ip++];
    uint8_t from = core->codePtr[core->ip++];
    uint8_t to = core->codePtr[core->ip++];

    ASSERT(reg < MAX_REG, "Invalid register index");
    ASSERT((from == 8 && to == 4), "Invalid byte sizes for floating-point downcasting");

    // Extract the double value from the register
    double doubleValue = 0.0;
    typev_memcpy(&doubleValue, &core->regs[reg], from);

    // Convert to float
    float floatValue = (float)doubleValue;

    // Store the result back in the register
    typev_memcpy(&core->regs[reg], &floatValue, to);
}



#define OP_BINARY(name, type, op)\
void name##_##type(TypeV_Core* core){\
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
OP_BINARY(add, u32, +)
OP_BINARY(add, i64, +)
OP_BINARY(add, u64, +)
OP_BINARY(add, f32, +)
OP_BINARY(add, f64, +)

void add_ptr_u8(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];
    uint8_t op2 = core->codePtr[core->ip++];
    core->regs[target].ptr = core->regs[op1].ptr + core->regs[op2].u8;
}

void add_ptr_u16(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];
    uint8_t op2 = core->codePtr[core->ip++];
    core->regs[target].ptr = core->regs[op1].ptr + core->regs[op2].u16;
}

void add_ptr_u32(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];
    uint8_t op2 = core->codePtr[core->ip++];
    core->regs[target].ptr = core->regs[op1].ptr + core->regs[op2].u32;
}

void add_ptr_u64(TypeV_Core* core){
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

void sub_ptr_u8(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];
    uint8_t op2 = core->codePtr[core->ip++];
    core->regs[target].ptr = core->regs[op1].ptr - core->regs[op2].u8;
}

void sub_ptr_u16(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];
    uint8_t op2 = core->codePtr[core->ip++];
    core->regs[target].ptr = core->regs[op1].ptr - core->regs[op2].u16;
}

void sub_ptr_u32(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];
    uint8_t op2 = core->codePtr[core->ip++];
    core->regs[target].ptr = core->regs[op1].ptr - core->regs[op2].u32;
}

void sub_ptr_u64(TypeV_Core* core){
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

void cmp_i8(TypeV_Core* core) {
    // Get first and second registers
    uint8_t op1 = core->codePtr[core->ip++];
    uint8_t op2 = core->codePtr[core->ip++];

    // Get both values as int8_t
    int8_t v1 = core->regs[op1].i8;
    int8_t v2 = core->regs[op2].i8;

    // Perform the comparison as int16_t
    int16_t result = (int16_t)(int)((int16_t)v1 - (int16_t)v2);

    // Set Zero Flag (ZF)
    WRITE_FLAG(*core->flags, FLAG_ZF, result==0);

    // Set Sign Flag (SF) using bit shifting
    WRITE_FLAG(*core->flags, FLAG_SF, (result >> 15) & 1);

    // Set Overflow Flag (OF) using bit shifting
    int v1_sign = (v1 >> 7) & 1;
    int v2_sign = (v2 >> 7) & 1;
    int result_sign = (result >> 15) & 1;
    WRITE_FLAG(*core->flags, FLAG_OF, (v1_sign != v2_sign) && (v1_sign != result_sign));

    // Set SignType Flag (STF) for signed operation
    WRITE_FLAG(*core->flags, FLAG_TF, 1); // Corrected to FLAG_STF

    // Clear Carry Flag (CF) as it is not relevant here
    FLAG_CLEAR(*core->flags, FLAG_CF);
}

void cmp_u8(TypeV_Core* core) {
    // Get first and second registers
    uint8_t op1 = core->codePtr[core->ip++];
    uint8_t op2 = core->codePtr[core->ip++];

    // Get both values as uint8_t
    uint8_t v1 = core->regs[op1].u8;
    uint8_t v2 = core->regs[op2].u8;

    // Perform the comparison as uint16_t to handle any potential overflow
    uint16_t result = (uint16_t)v1 - (uint16_t)v2;

    // Set Zero Flag (ZF) using bitwise operations
    WRITE_FLAG(*core->flags, FLAG_ZF, result==0);

    // Overflow Flag (OF) and Sign Flag (SF) are not used in unsigned comparisons
    // Clearing OF and SF for clarity
    WRITE_FLAG(*core->flags, FLAG_OF, 0);
    WRITE_FLAG(*core->flags, FLAG_SF, 0);

    // Set Carry Flag (CF) if there was a borrow in the subtraction
    WRITE_FLAG(*core->flags, FLAG_CF, v1 < v2);

    // Set SignType Flag (STF) for unsigned operation
    WRITE_FLAG(*core->flags, FLAG_TF, 0);
}

void cmp_i16(TypeV_Core* core) {
    // Get first and second registers
    uint8_t op1 = core->codePtr[core->ip++];
    uint8_t op2 = core->codePtr[core->ip++];

    // Get both values as int16_t
    int16_t v1 = core->regs[op1].i16;
    int16_t v2 = core->regs[op2].i16;

    // Perform the comparison as int32_t to handle overflow properly
    int32_t result = (int32_t)v1 - (int32_t)v2;

    // Set Zero Flag (ZF) using bitwise operations
    WRITE_FLAG(*core->flags, FLAG_ZF, result==0);

    // Set Sign Flag (SF) using bit shifting
    WRITE_FLAG(*core->flags, FLAG_SF, (result >> 31) & 1);

    // Set Overflow Flag (OF) using bit shifting
    int v1_sign = (v1 >> 15) & 1;
    int v2_sign = (v2 >> 15) & 1;
    int result_sign = (result >> 31) & 1;
    WRITE_FLAG(*core->flags, FLAG_OF, (v1_sign != v2_sign) && (v1_sign != result_sign));

    // Set SignType Flag (STF) for signed operation
    WRITE_FLAG(*core->flags, FLAG_TF, 1);
}

void cmp_u16(TypeV_Core* core) {
    // Get first and second registers
    uint8_t op1 = core->codePtr[core->ip++];
    uint8_t op2 = core->codePtr[core->ip++];

    // Get both values as uint16_t
    uint16_t v1 = core->regs[op1].u16;
    uint16_t v2 = core->regs[op2].u16;

    // Perform the comparison as uint32_t to handle any potential wrap-around or borrow
    uint32_t result = (uint32_t)v1 - (uint32_t)v2;

    // Set Zero Flag (ZF) using bitwise operations
    WRITE_FLAG(*core->flags, FLAG_ZF, result==0);

    // Overflow Flag (OF) and Sign Flag (SF) are not used in unsigned comparisons
    // Clearing OF and SF for clarity
    WRITE_FLAG(*core->flags, FLAG_OF, 0);
    WRITE_FLAG(*core->flags, FLAG_SF, 0);

    // Set Carry Flag (CF) if there was a borrow in the subtraction
    WRITE_FLAG(*core->flags, FLAG_CF, v1 < v2);

    // Set SignType Flag (STF) for unsigned operation
    WRITE_FLAG(*core->flags, FLAG_TF, 0);
}

void cmp_i32(TypeV_Core* core) {
    // Get first and second registers
    uint8_t op1 = core->codePtr[core->ip++];
    uint8_t op2 = core->codePtr[core->ip++];

    // Get both values as int32_t
    int32_t v1 = core->regs[op1].i32;
    int32_t v2 = core->regs[op2].i32;

    // Perform the comparison as int64_t to handle overflow properly
    int64_t result = (int64_t)v1 - (int64_t)v2;

    // Set Zero Flag (ZF) using bitwise operations
    WRITE_FLAG(*core->flags, FLAG_ZF, result==0);

    // Set Sign Flag (SF) using bit shifting
    WRITE_FLAG(*core->flags, FLAG_SF, (result >> 63) & 1);

    // Set Overflow Flag (OF) using bit shifting
    int v1_sign = (v1 >> 31) & 1;
    int v2_sign = (v2 >> 31) & 1;
    int64_t result_sign = (result >> 63) & 1;
    WRITE_FLAG(*core->flags, FLAG_OF, (v1_sign != v2_sign) && (v1_sign != result_sign));

    // Set SignType Flag (STF) for signed operation
    WRITE_FLAG(*core->flags, FLAG_TF, 1);
}

void cmp_u32(TypeV_Core* core) {
    // Get first and second registers
    uint8_t op1 = core->codePtr[core->ip++];
    uint8_t op2 = core->codePtr[core->ip++];

    // Get both values as uint32_t
    uint32_t v1 = core->regs[op1].u32;
    uint32_t v2 = core->regs[op2].u32;

    // Perform the comparison as uint64_t to handle wrap-around properly
    uint64_t result = (uint64_t)v1 - (uint64_t)v2;

    // Set Zero Flag (ZF) using bitwise operations
    WRITE_FLAG(*core->flags, FLAG_ZF, result==0);

    // Overflow Flag (OF) and Sign Flag (SF) are not used in unsigned comparisons
    // Clearing OF and SF for clarity
    WRITE_FLAG(*core->flags, FLAG_OF, 0);
    WRITE_FLAG(*core->flags, FLAG_SF, 0);

    // Set Carry Flag (CF) if there was a borrow in the subtraction
    WRITE_FLAG(*core->flags, FLAG_CF, v1 < v2);

    // Set SignType Flag (STF) for unsigned operation
    WRITE_FLAG(*core->flags, FLAG_TF, 0);
}

void cmp_i64(TypeV_Core* core) {
    // Get first and second registers
    uint8_t op1 = core->codePtr[core->ip++];
    uint8_t op2 = core->codePtr[core->ip++];

    // Get both values as int64_t
    int64_t v1 = core->regs[op1].i64;
    int64_t v2 = core->regs[op2].i64;

    // Detecting overflow in 64-bit integer subtraction can be complex
    // Overflow occurs if the sign of v1 is different from v2 AND
    // the sign of the result (v1 - v2) is different from v1

    // Direct subtraction for Zero and Sign flags
    int64_t result = v1 - v2;

    // Set Zero Flag (ZF)
    WRITE_FLAG(*core->flags, FLAG_ZF, result == 0);

    // Set Sign Flag (SF)
    WRITE_FLAG(*core->flags, FLAG_SF, result < 0);

    // Set Overflow Flag (OF)
    int v1_sign = v1 < 0;
    int v2_sign = v2 < 0;
    int result_sign = result < 0;
    WRITE_FLAG(*core->flags, FLAG_OF, (v1_sign != v2_sign) && (v1_sign != result_sign));

    // Set SignType Flag (STF) for signed operation
    WRITE_FLAG(*core->flags, FLAG_TF, 1);
}

void cmp_u64(TypeV_Core* core) {
    // Get first and second registers
    uint8_t op1 = core->codePtr[core->ip++];
    uint8_t op2 = core->codePtr[core->ip++];

    // Get both values as uint64_t
    uint64_t v1 = core->regs[op1].u64;
    uint64_t v2 = core->regs[op2].u64;

    // Perform the subtraction
    uint64_t result = v1 - v2;

    // Set Zero Flag (ZF)
    WRITE_FLAG(*core->flags, FLAG_ZF, result == 0);

    // Overflow Flag (OF) and Sign Flag (SF) are not relevant in unsigned comparisons
    // Clearing them for clarity
    WRITE_FLAG(*core->flags, FLAG_OF, 0);
    WRITE_FLAG(*core->flags, FLAG_SF, 0);

    // Set Carry Flag (CF) if there was a borrow in the subtraction
    WRITE_FLAG(*core->flags, FLAG_CF, v1 < v2);

    // Set SignType Flag (STF) for unsigned operation
    WRITE_FLAG(*core->flags, FLAG_TF, 0);
}

void cmp_f32(TypeV_Core* core) {
    // Get first and second registers
    uint8_t op1 = core->codePtr[core->ip++];
    uint8_t op2 = core->codePtr[core->ip++];

    // Get both values as float
    float v1 = core->regs[op1].f32;
    float v2 = core->regs[op2].f32;

    // Compare the float values
    float result = v1 - v2;

    // Set Zero Flag (ZF)
    WRITE_FLAG(*core->flags, FLAG_ZF, result == 0.0f);

    // Set Sign Flag (SF) - Negative if result is negative, clear otherwise
    WRITE_FLAG(*core->flags, FLAG_SF, result < 0.0f);

    // For floating-point comparison, Overflow Flag (OF) and Carry Flag (CF) are not applicable
    WRITE_FLAG(*core->flags, FLAG_OF, 0);
    WRITE_FLAG(*core->flags, FLAG_CF, 0);

    // Set SignType Flag (STF) for floating-point operation
    WRITE_FLAG(*core->flags, FLAG_TF, 1);
}

void cmp_f64(TypeV_Core* core) {
    // Get first and second registers
    uint8_t op1 = core->codePtr[core->ip++];
    uint8_t op2 = core->codePtr[core->ip++];

    // Get both values as double
    double v1 = core->regs[op1].f64;
    double v2 = core->regs[op2].f64;

    // Compare the double values
    double result = v1 - v2;

    // Set Zero Flag (ZF)
    WRITE_FLAG(*core->flags, FLAG_ZF, result == 0.0);

    // Set Sign Flag (SF) - Negative if result is negative, clear otherwise
    WRITE_FLAG(*core->flags, FLAG_SF, result < 0.0);

    // For floating-point comparison, Overflow Flag (OF) and Carry Flag (CF) are not applicable
    WRITE_FLAG(*core->flags, FLAG_OF, 0);
    WRITE_FLAG(*core->flags, FLAG_CF, 0);

    // Set SignType Flag (STF) for floating-point operation
    WRITE_FLAG(*core->flags, FLAG_TF, 1);
}

void cmp_ptr(TypeV_Core* core) {
    // Get first and second registers
    uint8_t op1 = core->codePtr[core->ip++];
    uint8_t op2 = core->codePtr[core->ip++];

    // Get both values as pointers (assuming 64-bit pointers)
    size_t ptr1 = core->regs[op1].ptr;
    size_t ptr2 = core->regs[op2].ptr;

    // Perform the comparison as uint64_t
    size_t result = ptr1 - ptr2;

    // Set Zero Flag (ZF)
    WRITE_FLAG(*core->flags, FLAG_ZF, result == 0);

    // Overflow Flag (OF) and Sign Flag (SF) are not relevant in pointer comparisons
    WRITE_FLAG(*core->flags, FLAG_OF, 0);
    WRITE_FLAG(*core->flags, FLAG_SF, 0);

    // Set Carry Flag (CF) if there was a "borrow" in the subtraction
    WRITE_FLAG(*core->flags, FLAG_CF, ptr1 < ptr2);

    // Set SignType Flag (STF) for unsigned operation
    WRITE_FLAG(*core->flags, FLAG_TF, 0);
}

void band_8(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];
    uint8_t op2 = core->codePtr[core->ip++];

    core->regs[target].u8 = core->regs[op1].u8 & core->regs[op2].u8;
}

void band_16(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];
    uint8_t op2 = core->codePtr[core->ip++];

    core->regs[target].u16 = core->regs[op1].u16 & core->regs[op2].u16;
}

void band_32(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];
    uint8_t op2 = core->codePtr[core->ip++];

    core->regs[target].u32 = core->regs[op1].u32 & core->regs[op2].u32;
}

void band_64(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];
    uint8_t op2 = core->codePtr[core->ip++];

    core->regs[target].u64 = core->regs[op1].u64 & core->regs[op2].u64;
}

void bor_8(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];
    uint8_t op2 = core->codePtr[core->ip++];

    core->regs[target].u8 = core->regs[op1].u8 | core->regs[op2].u8;
}

void bor_16(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];
    uint8_t op2 = core->codePtr[core->ip++];

    core->regs[target].u16 = core->regs[op1].u16 | core->regs[op2].u16;
}

void bor_32(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];
    uint8_t op2 = core->codePtr[core->ip++];

    core->regs[target].u32 = core->regs[op1].u32 | core->regs[op2].u32;
}

void bor_64(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];
    uint8_t op2 = core->codePtr[core->ip++];

    core->regs[target].u64 = core->regs[op1].u64 | core->regs[op2].u64;
}

void bxor_8(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];

    core->regs[target].u8 = core->regs[op1].u8 ^ core->regs[target].u8;
}

void bxor_16(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];

    core->regs[target].u16 = core->regs[op1].u16 ^ core->regs[target].u16;
}

void bxor_32(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];

    core->regs[target].u32 = core->regs[op1].u32 ^ core->regs[target].u32;
}

void bxor_64(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];

    core->regs[target].u64 = core->regs[op1].u64 ^ core->regs[target].u64;
}

void bnot_8(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];

    core->regs[target].u8 = ~core->regs[op1].u8;
}

void bnot_16(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];

    core->regs[target].u16 = ~core->regs[op1].u16;
}

void bnot_32(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];

    core->regs[target].u32 = ~core->regs[op1].u32;
}

void bnot_64(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];

    core->regs[target].u64 = ~core->regs[op1].u64;
}

void and(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];
    uint8_t op2 = core->codePtr[core->ip++];

    core->regs[target].u8 = core->regs[op1].u8 && core->regs[op2].u8;
}

void or(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];
    uint8_t op2 = core->codePtr[core->ip++];

    core->regs[target].u8 = core->regs[op1].u8 || core->regs[op2].u8;
}

void not(TypeV_Core* core){
    uint8_t target = core->codePtr[core->ip++];
    uint8_t op1 = core->codePtr[core->ip++];

    core->regs[target].u8 = !core->regs[op1].u8;
}

void jmp(TypeV_Core* core){
    const uint8_t offset_length = core->codePtr[core->ip++];
    size_t offset = 0; 
    typev_memcpy(&offset, &core->codePtr[core->ip],  offset_length);
    core->ip = offset;
}

void jmp_e(TypeV_Core* core){
    const uint8_t offset_length = core->codePtr[core->ip++];
    size_t offset = 0; 


    if(FLAG_CHECK(*core->flags, FLAG_ZF)){
        typev_memcpy(&offset, &core->codePtr[core->ip],  offset_length);
        core->ip = offset;
    }
    else {
        core->ip += offset_length;
    }
}

void jmp_ne(TypeV_Core* core){
    const uint8_t offset_length = core->codePtr[core->ip++];
    size_t offset = 0; 

    if(!FLAG_CHECK(*core->flags, FLAG_ZF)){
        typev_memcpy(&offset, &core->codePtr[core->ip],  offset_length);
        core->ip = offset;
    }
    else {
        core->ip += offset_length;
    }
}

void jmp_g(TypeV_Core* core){
    const uint8_t offset_length = core->codePtr[core->ip++];
    size_t offset = 0; 

    uint8_t can_jump = 0;
    // Check if the last operation was signed or unsigned
    if (FLAG_CHECK(*core->flags, FLAG_TF)) {
        // Signed comparison
        // Jump if ZF is clear, and either SF equals OF, or OF is clear and SF is clear
        can_jump = !FLAG_CHECK(*core->flags, FLAG_ZF) &&
                        ((FLAG_CHECK(*core->flags, FLAG_SF) == FLAG_CHECK(*core->flags, FLAG_OF)) ||
                         (!FLAG_CHECK(*core->flags, FLAG_OF) && !FLAG_CHECK(*core->flags, FLAG_SF)));
    } else {
        // Unsigned comparison
        // Jump if ZF is clear and CF is clear (no borrow occurred)
        can_jump = !FLAG_CHECK(*core->flags, FLAG_ZF) &&
                        !FLAG_CHECK(*core->flags, FLAG_CF);
    }

    if(can_jump){
        typev_memcpy(&offset, &core->codePtr[core->ip],  offset_length);
        core->ip = offset;
    }
    else {
        core->ip += offset_length;
    }
}

void jmp_ge(TypeV_Core* core){
    const uint8_t offset_length = core->codePtr[core->ip++];
    size_t offset = 0; 

    uint8_t can_jump = 0;
    if (FLAG_CHECK(*core->flags, FLAG_TF)) {
        // Signed comparison
        // Jump if either SF equals OF, or OF is clear and SF is clear
        can_jump = (FLAG_CHECK(*core->flags, FLAG_SF) == FLAG_CHECK(*core->flags, FLAG_OF)) ||
                        (!FLAG_CHECK(*core->flags, FLAG_OF) && !FLAG_CHECK(*core->flags, FLAG_SF));
    } else {
        // Unsigned comparison
        // Jump if CF is clear (no borrow occurred)
        can_jump = !FLAG_CHECK(*core->flags, FLAG_CF);
    }

    if(can_jump){
        typev_memcpy(&offset, &core->codePtr[core->ip],  offset_length);
        core->ip = offset;
    }
    else {
        core->ip += offset_length;
    }
}

void jmp_l(TypeV_Core* core){
    const uint8_t offset_length = core->codePtr[core->ip++];
    size_t offset = 0; 

    uint8_t can_jump = 0;
    if (FLAG_CHECK(*core->flags, FLAG_TF)) {
        // Signed comparison
        // Jump if SF does not equal OF
        can_jump = FLAG_CHECK(*core->flags, FLAG_SF) != FLAG_CHECK(*core->flags, FLAG_OF);
    } else {
        // Unsigned comparison
        // Jump if CF is set (borrow occurred)
        can_jump = FLAG_CHECK(*core->flags, FLAG_CF);
    }

    if(can_jump){
        typev_memcpy(&offset, &core->codePtr[core->ip],  offset_length);
        core->ip = offset;
    }
    else {
        core->ip += offset_length;
    }
}

void jmp_le(TypeV_Core* core){
    const uint8_t offset_length = core->codePtr[core->ip++];
    size_t offset = 0; 


    uint8_t can_jump = 0;
    if (FLAG_CHECK(*core->flags, FLAG_TF)) {
        // Signed comparison
        // Jump if ZF is set, or SF does not equal OF
        can_jump = FLAG_CHECK(*core->flags, FLAG_ZF) ||
                        (FLAG_CHECK(*core->flags, FLAG_SF) != FLAG_CHECK(*core->flags, FLAG_OF));
    } else {
        // Unsigned comparison
        // Jump if CF is set (borrow occurred), or ZF is set
        can_jump = FLAG_CHECK(*core->flags, FLAG_CF) ||
                        FLAG_CHECK(*core->flags, FLAG_ZF);
    }

    if(can_jump){
        typev_memcpy(&offset, &core->codePtr[core->ip],  offset_length);
        core->ip = offset;
    }
    else {
        core->ip += offset_length;
    }
}

void reg_ffi(TypeV_Core* core){
    uint8_t offsetSize = core->codePtr[core->ip++];
    size_t offset = 0;
    typev_memcpy(&offset, &core->codePtr[core->ip], offsetSize);
    core->ip += offsetSize;

    uint16_t id = 0;
    typev_memcpy(&id, &core->codePtr[core->ip], 2);
    core->ip += 2;

    char* namePtr = &core->constPtr[offset];
    char* name = strdup(namePtr);
    engine_ffi_register(core->engineRef, name, id);
}

void open_ffi(TypeV_Core* core){
    uint16_t ffi_id = 0;
    typev_memcpy(&ffi_id, &core->codePtr[core->ip], 2);
    core->ip += 2;

    engine_ffi_open(core->engineRef, ffi_id);
}

void ld_ffi(TypeV_Core* core){
    uint8_t dest = core->codePtr[core->ip++];
    ASSERT(dest < MAX_REG, "Invalid register index");

    uint16_t id = 0;
    typev_memcpy(&id, &core->codePtr[core->ip], 2);
    core->ip += 2;

    uint8_t methodId = core->codePtr[core->ip++];

    core->regs[dest].ptr = (size_t)engine_ffi_get(core->engineRef, id, methodId);
}

void call_ffi(TypeV_Core* core){
    uint8_t reg = core->codePtr[core->ip++];

    TypeV_FFIFunc ffi_fn = (TypeV_FFIFunc)(core->regs[reg].ptr);
    ffi_fn(core);
}

void close_ffi(TypeV_Core* core){
    uint8_t reg = core->codePtr[core->ip++];
    core_ffi_close(core, core->regs[reg].ptr);
}

void p_alloc(TypeV_Core* core){
    uint8_t reg = core->codePtr[core->ip++];
    uint8_t size_length = core->codePtr[core->ip++];
    size_t size = 0;
    typev_memcpy(&size, &core->codePtr[core->ip], size_length);
    core->ip += size_length;

    core->regs[reg].ptr = (size_t)engine_spawnCore(core->engineRef, core, size);
}

void p_dequeue(TypeV_Core* core){
    LOG_INFO("Core[%d] dequeueing 1/%d", core->id, core->messageInputQueue.length);
    uint8_t dest = core->codePtr[core->ip++];
    uint8_t promiseDest = core->codePtr[core->ip++];
    ASSERT(dest < MAX_REG, "Invalid register index");
    ASSERT(promiseDest < MAX_REG, "Invalid register index");

    // get the next queue element from core queue
    // check queue length
    if(core->messageInputQueue.length == 0) {
        LOG_ERROR("Core[%d] tried to dequeue from empty queue", core->id);
        ASSERT(0, "fail");
    }
    TypeV_IOMessage* msg = queue_dequeue(&core->messageInputQueue);
    core->regs[dest].ptr = (size_t)msg->message;
    core->regs[promiseDest].ptr = (size_t)msg->promise;
}

void p_emit(TypeV_Core* core){
    uint8_t targetProcessReg = core->codePtr[core->ip++];
    uint8_t dataReg = core->codePtr[core->ip++];
    uint8_t promiseReg = core->codePtr[core->ip++];

    ASSERT(targetProcessReg < MAX_REG, "Invalid register index");
    ASSERT(dataReg < MAX_REG, "Invalid register index");
    ASSERT(promiseReg < MAX_REG, "Invalid register index");

    size_t data_ptr = core->regs[dataReg].ptr;
    TypeV_Core* target = (TypeV_Core*)core->regs[targetProcessReg].ptr;

    LOG_INFO("Core[%d] emitting message to Core[%d]", core->id, target->id);

    TypeV_IOMessage* msg = malloc(sizeof(TypeV_IOMessage));
    msg->sender = core->id;
    msg->message = (void*)data_ptr;
    msg->promise = core_promise_alloc(core);
    core->regs[promiseReg].ptr = (size_t)msg->promise;

    core_enqueue_message(target, msg);

    core->regs[promiseReg].ptr = (size_t)msg->promise;
}

void p_wait_queue(TypeV_Core* core){
    LOG_INFO("Core[%d] waiting for queue", core->id);
    if(core->messageInputQueue.length == 0){
        core_queue_await(core);
    }
}

void p_queue_size(TypeV_Core* core){
    uint8_t dest = core->codePtr[core->ip++];
    core->regs[dest].u64 = core->messageInputQueue.length;
}

void p_send_sig(TypeV_Core * core){
    uint8_t targetProcessReg = core->codePtr[core->ip++];
    uint8_t sig = core->codePtr[core->ip++];

    ASSERT(targetProcessReg < MAX_REG, "Invalid register index");
    ASSERT(sig <= CSIG_KILL, "Invalid signal value");

    TypeV_Core* target = (TypeV_Core*)core->regs[targetProcessReg].ptr;

    LOG_INFO("Core[%d] sending signal %d to Core[%d]", core->id, sig, target->id);

    core_receive_signal(target, sig);
}

void p_id(TypeV_Core* core){
    uint8_t dest = core->codePtr[core->ip++];
    uint8_t processReg = core->codePtr[core->ip++];
    ASSERT(dest < MAX_REG, "Invalid register index");
    ASSERT(processReg < MAX_REG, "Invalid register index");

    TypeV_Core* c = (TypeV_Core*)core->regs[processReg].ptr;

    core->regs[dest].u32 = c->id;
}

void p_cid(TypeV_Core* core){
    uint8_t dest = core->codePtr[core->ip++];
    ASSERT(dest < MAX_REG, "Invalid register index");
    core->regs[dest].u32 = core->id;
}

void p_state(TypeV_Core* core){
    uint8_t dest = core->codePtr[core->ip++];
    uint8_t processReg = core->codePtr[core->ip++];
    ASSERT(dest < MAX_REG, "Invalid register index");
    ASSERT(processReg < MAX_REG, "Invalid register index");

    TypeV_Core* c = (TypeV_Core*)core->regs[processReg].ptr;

    core->regs[dest].u8 = c->state;
}

void promise_alloc(TypeV_Core* core){
    uint8_t dest = core->codePtr[core->ip++];
    ASSERT(dest < MAX_REG, "Invalid register index");
    core->regs[dest].ptr = (size_t) core_promise_alloc(core);
}

void promise_resolve(TypeV_Core* core){
    uint8_t promiseReg = core->codePtr[core->ip++];
    uint8_t dataReg = core->codePtr[core->ip++];
    ASSERT(promiseReg < MAX_REG, "Invalid register index");
    ASSERT(dataReg < MAX_REG, "Invalid register index");

    TypeV_Promise* promise = (TypeV_Promise*)core->regs[promiseReg].ptr;
    size_t data = core->regs[dataReg].ptr;

    core_promise_resolve(core, promise, data);
}

void promise_await(TypeV_Core* core){
    //LOG_INFO("Core[%d] awaiting promise", core->id);
    uint8_t promiseReg = core->codePtr[core->ip++];
    ASSERT(promiseReg < MAX_REG, "Invalid register index");

    TypeV_Promise* promise = (TypeV_Promise*)core->regs[promiseReg].ptr;

    core_promise_await(core, promise);
}

void promise_data(TypeV_Core* core){
    uint8_t dest = core->codePtr[core->ip++];
    uint8_t promiseReg = core->codePtr[core->ip++];
    ASSERT(dest < MAX_REG, "Invalid register index");
    ASSERT(promiseReg < MAX_REG, "Invalid register index");

    TypeV_Promise* promise = (TypeV_Promise*)core->regs[promiseReg].ptr;

    core->regs[dest].ptr = (size_t)(promise->value);
}

void lock_alloc(TypeV_Core* core){
    LOG_INFO("Core[%d] allocating lock", core->id);
    uint8_t destReg = core->codePtr[core->ip++];
    uint8_t dataReg = core->codePtr[core->ip++];
    ASSERT(destReg < MAX_REG, "Invalid register index");
    ASSERT(dataReg < MAX_REG, "Invalid register index");
    core->regs[destReg].ptr = (size_t) core_lock_alloc(core, core->regs[dataReg].ptr);
}

void lock_acquire(TypeV_Core* core){
    uint8_t lockReg = core->codePtr[core->ip++];
    uint8_t dataReg = core->codePtr[core->ip++];
    ASSERT(lockReg < MAX_REG, "Invalid register index");
    ASSERT(dataReg < MAX_REG, "Invalid register index");
    TypeV_Lock* lock = (TypeV_Lock*)core->regs[lockReg].ptr;
    LOG_INFO("Core[%d] attempting to acquire lock %d, isLocked %d:", core->id, lock->id, lock->locked);

    if(lock->locked){
        LOG_INFO("Core[%d] waiting for lock, moving IP backwards to the lock_acquire, to be ready to acquire lock later.", core->id);
        // go back to the main instruction to await the lock
        core->ip -= 3;
        core_lock_acquire(core, lock);
    }
    else {
        core_lock_acquire(core, lock);
        core->regs[dataReg].ptr = lock->value;
    }
}

void lock_release(TypeV_Core* core){
    uint8_t lockReg = core->codePtr[core->ip++];
    ASSERT(lockReg < MAX_REG, "Invalid register index");
    TypeV_Lock* lock = (TypeV_Lock*)core->regs[lockReg].ptr;
    LOG_INFO("Core[%d] Releasing lock %d", core->id, lock->id);

    core_lock_release(core, lock);
}



void debug_reg(TypeV_Core* core){
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

void halt(TypeV_Core* core) {
    uint8_t code_reg = core->codePtr[core->ip++];
    ASSERT(code_reg < MAX_REG, "Invalid register index");

    uint32_t code = core->regs[code_reg].u32;

    core->isRunning = 0;
    core->state = CS_TERMINATED;
    core->exitCode = code;
    engine_detach_core(core->engineRef, core);
}

void load_std(TypeV_Core* core){
    TypeV_StdLibs* stdLibs = typev_std_get_libs();

    uint8_t lib_id = core->codePtr[core->ip++];
    uint8_t fn_id = core->codePtr[core->ip++];

    ASSERT(lib_id < stdLibs->len, "Invalid stdlib index");
    ASSERT(fn_id < stdLibs->libs[lib_id].len, "Invalid stdlib function index");

    stdLibs->libs[lib_id].fns[fn_id](core);
}

void vm_health(TypeV_Core* core){
    uint8_t dest = core->codePtr[core->ip++];
    ASSERT(dest < MAX_REG, "Invalid register index");

    core->regs[dest].u8 = core->engineRef->health;
}


