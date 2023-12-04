//
// Created by praisethemoon on 21.11.23.
//

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


void mv_reg_reg(TypeV_Core* core){
    const uint8_t target = core->program.bytecode[core->registers.ip++];
    const uint8_t source = core->program.bytecode[core->registers.ip++];
    uint8_t bytesize = core->program.bytecode[core->registers.ip++];
    if(bytesize == 0) bytesize = PTR_SIZE;

    ASSERT(target < MAX_REG, "Invalid register index");
    ASSERT(source < MAX_REG, "Invalid register index");
    ASSERT(bytesize <= 8, "Invalid bit size");
    memcpy(&core->registers.regs[target], &core->registers.regs[source], bytesize);
}

void mv_reg_i(TypeV_Core* core){
    const uint8_t target = core->program.bytecode[core->registers.ip++];
    uint8_t immediate_size = core->program.bytecode[core->registers.ip++];
    uint64_t immediate = 0;
    memcpy(&immediate, &core->program.bytecode[core->registers.ip], immediate_size);
    core->registers.ip += immediate_size;
    memcpy(&core->registers.regs[target], &immediate, 8);
}


#define mv_reg_const(bits, size) \
void mv_reg_const_##bits(TypeV_Core* core){\
    const uint8_t target = core->program.bytecode[core->registers.ip++];\
    const uint8_t offset_length = core->program.bytecode[core->registers.ip++];\
    size_t constant_offset = 0; /* we do not increment offset here*/\
    memcpy(&constant_offset, &core->program.bytecode[core->registers.ip],  offset_length);\
    core->registers.ip += offset_length;\
    memcpy(&core->registers.regs[target], &core->constantPool.pool[constant_offset], size);\
}

void mv_reg_const_8(TypeV_Core* core){
    const uint8_t target = core->program.bytecode[core->registers.ip++];
    const uint8_t offset_length = core->program.bytecode[core->registers.ip++];
    size_t constant_offset = 0; /* we do not increment offset here*/
    memcpy(&constant_offset, &core->program.bytecode[core->registers.ip],  offset_length);
    core->registers.ip += offset_length;
    memcpy(&core->registers.regs[target], &core->constantPool.pool[constant_offset], 1);
}

//mv_reg_const(8, 1)
mv_reg_const(16, 2)
mv_reg_const(32, 4)
mv_reg_const(64, 8)
mv_reg_const(ptr, PTR_SIZE)
#undef mv_reg_const

void mv_reg_mem(TypeV_Core* core){
    const uint8_t target = core->program.bytecode[core->registers.ip++];
    const uint8_t source = core->program.bytecode[core->registers.ip++];
    uint8_t bytesize = core->program.bytecode[core->registers.ip++];
    if(bytesize == 0) bytesize = PTR_SIZE;

    ASSERT(target < MAX_REG, "Invalid register index");
    ASSERT(source < MAX_REG, "Invalid register index");
    ASSERT(bytesize <= 8, "Invalid bit size");
    memcpy(&core->registers.regs[target], &core->stack.stack[core->registers.regs[source].u64], bytesize);
}

void mv_mem_reg(TypeV_Core* core){
    const uint8_t target = core->program.bytecode[core->registers.ip++];
    const uint8_t source = core->program.bytecode[core->registers.ip++];
    uint8_t bytesize = core->program.bytecode[core->registers.ip++];
    if(bytesize == 0) bytesize = PTR_SIZE;

    ASSERT(target < MAX_REG, "Invalid register index");
    ASSERT(source < MAX_REG, "Invalid register index");
    ASSERT(bytesize <= 8, "Invalid bit size");

    void* ptr = (void*)core->registers.regs[target].ptr;

    memcpy(&ptr, &core->registers.regs[source], bytesize);
}

#define MV_REG_LOCAL(bits, size)\
void mv_reg_local_##bits(TypeV_Core* core){\
    const uint8_t target = core->program.bytecode[core->registers.ip++];\
    const uint8_t offset_length = core->program.bytecode[core->registers.ip++];\
    size_t offset = 0; /* we do not increment offset here*/\
    memcpy(&offset, &core->program.bytecode[core->registers.ip],  offset_length);\
    core->registers.ip += offset_length;\
    ASSERT(target < MAX_REG, "Invalid register index");\
    ASSERT(offset < core->stack.capacity, "Invalid stack offset");\
    memcpy(&core->registers.regs[target], &core->stack.stack[core->registers.fp+offset], size);\
}

MV_REG_LOCAL(8, 1)
MV_REG_LOCAL(16, 2)
MV_REG_LOCAL(32, 4)
MV_REG_LOCAL(64, 8)
MV_REG_LOCAL(ptr, PTR_SIZE)
#undef MV_REG_LOCAL

#define MV_LOCAL_REG(bits, size) \
void mv_local_reg_##bits(TypeV_Core* core){\
    const uint8_t offset_length = core->program.bytecode[core->registers.ip++];\
    size_t offset = 0; /* we do not increment offset here*/\
    memcpy(&offset, &core->program.bytecode[core->registers.ip],  offset_length);\
    core->registers.ip += offset_length;\
    const uint8_t source = core->program.bytecode[core->registers.ip++];\
    ASSERT(source < MAX_REG, "Invalid register index");\
    ASSERT(offset < core->stack.capacity, "Invalid stack offset");\
    memcpy(&core->stack.stack[core->registers.fp+offset], &core->registers.regs[source], size);\
}

MV_LOCAL_REG(8, 1)
MV_LOCAL_REG(16, 2)
MV_LOCAL_REG(32, 4)
MV_LOCAL_REG(64, 8)
MV_LOCAL_REG(ptr, PTR_SIZE)
#undef MV_LOCAL_REG

#define MV_GLOBAL_REG(bits, size) \
void mv_global_reg_##bits(TypeV_Core* core){\
    const uint8_t offset_length = core->program.bytecode[core->registers.ip++];\
    size_t offset = 0; /* we do not increment offset here*/\
    memcpy(&offset, &core->program.bytecode[core->registers.ip],  offset_length);\
    core->registers.ip += offset_length;\
    const uint8_t source = core->program.bytecode[core->registers.ip++];\
    ASSERT(source < MAX_REG, "Invalid register index");\
    ASSERT(offset < core->globalPool.length, "Invalid global offset");         \
    memcpy(&core->globalPool.pool[offset], &core->registers.regs[source], size);\
}

MV_GLOBAL_REG(8, 1)
MV_GLOBAL_REG(16, 2)
MV_GLOBAL_REG(32, 4)
//MV_GLOBAL_REG(64, 8)
void mv_global_reg_64(TypeV_Core* core){
    const uint8_t offset_length = core->program.bytecode[core->registers.ip++];
    size_t offset = 0; /* we do not increment offset here*/
    memcpy(&offset, &core->program.bytecode[core->registers.ip],  offset_length);
    core->registers.ip += offset_length;
    const uint8_t source = core->program.bytecode[core->registers.ip++];
    ASSERT(source < MAX_REG, "Invalid register index");
    ASSERT(offset < core->globalPool.length, "Invalid global offset");
    memcpy(&core->globalPool.pool[offset], &core->registers.regs[source], 8);
}
MV_GLOBAL_REG(ptr, PTR_SIZE)
#undef MV_GLOBAL_REG

#define MV_REG_GLOBAL(bits, size) \
void mv_reg_global_##bits(TypeV_Core* core){\
    const uint8_t target = core->program.bytecode[core->registers.ip++];\
    const uint8_t offset_length = core->program.bytecode[core->registers.ip++];\
    size_t offset = 0; /* we do not increment offset here*/\
    memcpy(&offset, &core->program.bytecode[core->registers.ip],  offset_length);\
    core->registers.ip += offset_length;\
    ASSERT(target < MAX_REG, "Invalid register index");\
    ASSERT(offset < core->globalPool.length, "Invalid global offset");\
    memcpy(&core->registers.regs[target], &core->globalPool.pool[offset], size);\
}

MV_REG_GLOBAL(8, 1)
MV_REG_GLOBAL(16, 2)
MV_REG_GLOBAL(32, 4)
//MV_REG_GLOBAL(64, 8)
void mv_reg_global_64(TypeV_Core* core){
    const uint8_t target = core->program.bytecode[core->registers.ip++];
    const uint8_t offset_length = core->program.bytecode[core->registers.ip++];
    size_t offset = 0; /* we do not increment offset here*/
    memcpy(&offset, &core->program.bytecode[core->registers.ip],  offset_length);
    core->registers.ip += offset_length;
    ASSERT(target < MAX_REG, "Invalid register index");
    ASSERT(offset < core->globalPool.length, "Invalid global offset");
    memcpy(&core->registers.regs[target], &core->globalPool.pool[offset], 8);
}
MV_REG_GLOBAL(ptr, PTR_SIZE)
#undef MV_REG_GLOBAL

#define MV_REG_ARG(bits, size) \
void mv_reg_arg_##bits(TypeV_Core* core){\
    const uint8_t target = core->program.bytecode[core->registers.ip++];\
    const uint8_t offset_length = core->program.bytecode[core->registers.ip++];\
    size_t offset = 0; /* we do not increment offset here*/\
    memcpy(&offset, &core->program.bytecode[core->registers.ip],  offset_length);\
    core->registers.ip += offset_length;\
    ASSERT(target < MAX_REG, "Invalid register index");\
    ASSERT(offset < core->stack.capacity, "Invalid stack offset");\
    memcpy(&core->registers.regs[target], &core->stack.stack[core->registers.fp+offset], size);\
}

void mv_reg_arg_16(TypeV_Core* core){
    const uint8_t target = core->program.bytecode[core->registers.ip++];
    const uint8_t offset_length = core->program.bytecode[core->registers.ip++];
    size_t offset = 0; /* we do not increment offset here*/
    memcpy(&offset, &core->program.bytecode[core->registers.ip],  offset_length);
    core->registers.ip += offset_length;
    ASSERT(target < MAX_REG, "Invalid register index");
    ASSERT(offset < core->stack.capacity, "Invalid stack offset");
    memcpy(&core->registers.regs[target], &core->stack.stack[core->registers.fp+offset], 2);
}


MV_REG_ARG(8, 1)
//MV_REG_ARG(16, 2)
MV_REG_ARG(32, 4)
MV_REG_ARG(64, 8)
MV_REG_ARG(ptr, PTR_SIZE)
#undef MV_REG_ARG

#define MV_ARG_REG(bits, size) \
void mv_arg_reg_##bits(TypeV_Core* core){\
    const uint8_t source = core->program.bytecode[core->registers.ip++];\
    const uint8_t offset_length = core->program.bytecode[core->registers.ip++];\
    size_t offset = 0; /* we do not increment offset here*/\
    memcpy(&offset, &core->program.bytecode[core->registers.ip],  offset_length);\
    core->registers.ip += offset_length;\
    ASSERT(source < MAX_REG, "Invalid register index");\
    ASSERT(offset < core->stack.capacity, "Invalid stack offset");\
    memcpy(&core->stack.stack[core->registers.fp+offset], &core->registers.regs[source], size);\
}

MV_ARG_REG(8, 1)
MV_ARG_REG(16, 2)
MV_ARG_REG(32, 4)
MV_ARG_REG(64, 8)
MV_ARG_REG(ptr, PTR_SIZE)
#undef MV_ARG_REG

void s_alloc(TypeV_Core* core){
    const uint8_t fields_count = core->program.bytecode[core->registers.ip++];
    const uint8_t struct_size_length = core->program.bytecode[core->registers.ip++];
    size_t struct_size = 0; /* we do not increment offset here*/
    memcpy(&struct_size, &core->program.bytecode[core->registers.ip],  struct_size_length);
    core->registers.ip += struct_size_length;

    // allocate memory for struct
    size_t mem = core_struct_alloc(core, fields_count, struct_size);
    // move the pointer to R16
    core->registers.regs[16].ptr = mem;
}

void s_alloc_shadow(TypeV_Core* core){
    const uint8_t fields_count = core->program.bytecode[core->registers.ip++];

    size_t original_mem = core->registers.regs[16].ptr;

    // allocate memory for struct
    size_t mem = core_struct_alloc_shadow(core, fields_count, original_mem);
    // move the pointer to R16
    core->registers.regs[16].ptr = mem;
}

void s_set_offset(TypeV_Core* core){
    const uint8_t field_index = core->program.bytecode[core->registers.ip++];
    const uint8_t offset_length = core->program.bytecode[core->registers.ip++];
    size_t offset = 0; /* we do not increment offset here*/
    memcpy(&offset, &core->program.bytecode[core->registers.ip],  offset_length);
    core->registers.ip += offset_length;

    TypeV_Struct* struct_ptr = (TypeV_Struct*)core->registers.regs[16].ptr;
    struct_ptr->fieldOffsets[field_index] = offset;
}

void s_loadf(TypeV_Core* core){
    const uint8_t target = core->program.bytecode[core->registers.ip++];
    const uint8_t field_index = core->program.bytecode[core->registers.ip++];
    uint8_t bytesize = core->program.bytecode[core->registers.ip++];
    if(bytesize == 0) bytesize = PTR_SIZE;
    ASSERT(target < MAX_REG, "Invalid register index");
    TypeV_Struct* struct_ptr = (TypeV_Struct*)core->registers.regs[16].ptr;
    memcpy(&core->registers.regs[target], struct_ptr->data+struct_ptr->fieldOffsets[field_index], bytesize);
}


#define S_STOREF_CONST(bits, size) \
void s_storef_const_##bits(TypeV_Core* core){\
    const uint8_t field_index = core->program.bytecode[core->registers.ip++];\
    const uint8_t offset_length = core->program.bytecode[core->registers.ip++];\
    size_t offset = 0; /* we do not increment offset here */\
    memcpy(&offset, &core->program.bytecode[core->registers.ip],  offset_length);\
    core->registers.ip += offset_length;\
    ASSERT(offset < core->constantPool.length, "Invalid constant offset");\
    TypeV_Struct* struct_ptr = (TypeV_Struct*)core->registers.regs[16].ptr;\
    memcpy(struct_ptr->data+struct_ptr->fieldOffsets[field_index], &core->constantPool.pool[offset], size);\
}

S_STOREF_CONST(8, 1)
S_STOREF_CONST(16, 2)
S_STOREF_CONST(32, 4)
S_STOREF_CONST(64, 8)
S_STOREF_CONST(ptr, PTR_SIZE)
#undef S_STOREF_CONST

void s_storef_reg(TypeV_Core* core){
    const uint8_t field_index = core->program.bytecode[core->registers.ip++];
    const uint8_t source = core->program.bytecode[core->registers.ip++];
    uint8_t bytesize = core->program.bytecode[core->registers.ip++];
    if(bytesize == 0) bytesize = PTR_SIZE;

    ASSERT(source < MAX_REG, "Invalid register index");
    ASSERT(bytesize <= 8, "Invalid byte size");

    TypeV_Struct* struct_ptr = (TypeV_Struct*)core->registers.regs[16].ptr;
    memcpy(struct_ptr->data+struct_ptr->fieldOffsets[field_index], &core->registers.regs[source], bytesize);
}

void c_allocf(TypeV_Core* core){
    const uint8_t fields_count = core->program.bytecode[core->registers.ip++];
    const uint8_t struct_size_length = core->program.bytecode[core->registers.ip++];
    size_t struct_size = 0; /* we do not increment offset here*/
    memcpy(&struct_size, &core->program.bytecode[core->registers.ip],  struct_size_length);
    core->registers.ip += struct_size_length;

    // allocate memory for struct
    size_t mem = core_class_alloc_fields(core, fields_count, struct_size);
    // move the pointer to R17
    core->registers.regs[17].ptr = mem;
}

void c_allocm(TypeV_Core* core){
    const uint8_t methods_count = core->program.bytecode[core->registers.ip++];
    // allocate memory for struct
    TypeV_Class* c = (TypeV_Class*)core->registers.regs[17].ptr;
    core_class_alloc_methods(core, methods_count, c);
}

void c_storem(TypeV_Core* core){
    const uint8_t method_index = core->program.bytecode[core->registers.ip++];
    const uint8_t offset_length = core->program.bytecode[core->registers.ip++];
    size_t offset = 0; /* we do not increment offset here*/
    memcpy(&offset, &core->program.bytecode[core->registers.ip],  offset_length);
    core->registers.ip += offset_length;

    TypeV_Class* c = (TypeV_Class*)core->registers.regs[17].ptr;
    LOG_INFO("Storing method %d at offset %d in class %p", method_index, offset, (void*)c);
    c->methodsOffset[method_index] = offset;
}

void c_loadm(TypeV_Core* core){
    const uint8_t target = core->program.bytecode[core->registers.ip++];
    const uint8_t method_index = core->program.bytecode[core->registers.ip++];

    ASSERT(target < MAX_REG, "Invalid register index");

    TypeV_Class* c = (TypeV_Class*)core->registers.regs[17].ptr;

    LOG_INFO("Loading method %d from class %p", method_index, (void*)c);

    size_t offset = c->methodsOffset[method_index];
    core->registers.regs[target].ptr = offset;
}

void c_storef_reg(TypeV_Core* core){
    const uint8_t field_index = core->program.bytecode[core->registers.ip++];
    const uint8_t source = core->program.bytecode[core->registers.ip++];
    uint8_t bytesize = core->program.bytecode[core->registers.ip++];
    if(bytesize == 0) bytesize = PTR_SIZE;
    ASSERT(source < MAX_REG, "Invalid register index");
    ASSERT(bytesize <= 8, "Invalid byte size");
    TypeV_Class* c = (TypeV_Class*)core->registers.regs[17].ptr;
    memcpy(c->data+c->fieldsOffset[field_index], &core->registers.regs[source], bytesize);
}


#define C_STOREF_CONST(bits, size) \
void c_storef_const_##bits(TypeV_Core* core){\
    const uint8_t field_index = core->program.bytecode[core->registers.ip++];\
    const uint8_t offset_length = core->program.bytecode[core->registers.ip++];\
    size_t offset = 0; /* we do not increment offset here */\
    memcpy(&offset, &core->program.bytecode[core->registers.ip],  offset_length);\
    core->registers.ip += offset_length;\
    ASSERT(offset < core->constantPool.length, "Invalid constant offset");\
    TypeV_Class* c = (TypeV_Class*)core->registers.regs[17].ptr;             \
    memcpy(c->data+c->fieldsOffset[field_index], &core->constantPool.pool[offset], size);\
}

C_STOREF_CONST(8, 1)
C_STOREF_CONST(16, 2)
C_STOREF_CONST(32, 4)
C_STOREF_CONST(64, 8)
C_STOREF_CONST(ptr, PTR_SIZE)
#undef C_STOREF_CONST

void c_loadf(TypeV_Core* core){
    const uint8_t target = core->program.bytecode[core->registers.ip++];
    const uint8_t field_index = core->program.bytecode[core->registers.ip++];
    uint8_t bytesize = core->program.bytecode[core->registers.ip++];
    if(bytesize == 0) bytesize = PTR_SIZE;
    ASSERT(target < MAX_REG, "Invalid register index");
    TypeV_Class* c = (TypeV_Class*)core->registers.regs[17].ptr;
    memcpy(&core->registers.regs[target], c->data+c->fieldsOffset[field_index], bytesize);
}


void i_alloc(TypeV_Core* core){
    const uint8_t fields_count = core->program.bytecode[core->registers.ip++];
    const uint8_t struct_size_length = core->program.bytecode[core->registers.ip++];
    size_t struct_size = 0; /* we do not increment offset here*/
    memcpy(&struct_size, &core->program.bytecode[core->registers.ip],  struct_size_length);
    core->registers.ip += struct_size_length;

    TypeV_Class* c = (TypeV_Class*)core->registers.regs[17].ptr;
    // allocate memory for struct
    size_t mem = core_interface_alloc(core, fields_count, c);
    // move the pointer to R18
    core->registers.regs[18].ptr = mem;
}

void i_set_offset(TypeV_Core* core){
    const uint8_t field_index = core->program.bytecode[core->registers.ip++];
    const uint8_t offset_length = core->program.bytecode[core->registers.ip++];
    size_t offset = 0; /* we do not increment offset here*/
    memcpy(&offset, &core->program.bytecode[core->registers.ip],  offset_length);
    core->registers.ip += offset_length;

    TypeV_Interface* i = (TypeV_Interface*)core->registers.regs[18].ptr;
    i->methodsOffset[field_index] = offset;
}

void i_loadm(TypeV_Core* core){
    const uint8_t target = core->program.bytecode[core->registers.ip++];
    const uint8_t method_index = core->program.bytecode[core->registers.ip++];

    ASSERT(target < MAX_REG, "Invalid register index");

    TypeV_Interface* i = (TypeV_Interface*)core->registers.regs[18].ptr;
    size_t offset = i->methodsOffset[method_index];

    core->registers.regs[target].ptr = i->classPtr->methods[offset];
}

void a_alloc(TypeV_Core* core){
    const uint8_t num_elements_size = core->program.bytecode[core->registers.ip++];
    size_t num_elements = 0; /* we do not increment offset here*/
    memcpy(&num_elements, &core->program.bytecode[core->registers.ip],  num_elements_size);
    core->registers.ip += num_elements_size;
    // next read the element size
    uint8_t element_size = core->program.bytecode[core->registers.ip++];
    ASSERT(element_size <= 8, "Invalid byte size");
    if(element_size == 0) element_size = PTR_SIZE;

    // allocate memory for struct
    size_t mem = core_array_alloc(core, num_elements, element_size);
    // move the pointer to R19
    core->registers.regs[19].ptr = mem;
}

void a_extend(TypeV_Core* core){
    const uint8_t num_elements_size = core->program.bytecode[core->registers.ip++];
    size_t num_elements = 0; /* we do not increment offset here*/
    memcpy(&num_elements, &core->program.bytecode[core->registers.ip],  num_elements_size);
    core->registers.ip += num_elements_size;

    size_t mem = core_array_extend(core, core->registers.regs[19].ptr, num_elements);
    core->registers.regs[19].ptr = mem;
}

void a_storef_reg(TypeV_Core* core){
    const uint8_t source = core->program.bytecode[core->registers.ip++];
    const uint8_t index = core->program.bytecode[core->registers.ip++];
    uint8_t element_size = core->program.bytecode[core->registers.ip++];
    if(element_size == 0) element_size = PTR_SIZE;
    ASSERT(source < MAX_REG, "Invalid register index");
    ASSERT(index < MAX_REG, "Invalid register index");
    ASSERT(element_size <= 8, "Invalid byte size");

    //LOG_INFO("Storing %d bytes at index %d", element_size, core->registers.regs[index].u64);

    TypeV_Array* array = (TypeV_Array*)core->registers.regs[19].ptr;
    memcpy(array->data+(core->registers.regs[index].u64*array->elementSize), &core->registers.regs[source], element_size);
}

#define A_STOREF_CONST(bits, size) \
void a_storef_const_##bits(TypeV_Core* core){\
    uint8_t indexReg = core->program.bytecode[core->registers.ip++];\
    uint8_t offset_size = core->program.bytecode[core->registers.ip++];\
    size_t offset = 0;\
    memcpy(&offset, &core->program.bytecode[core->registers.ip], offset_size);\
    \
    TypeV_Array* array = (TypeV_Array*)core->registers.regs[19].ptr;\
    memcpy(array->data+(core->registers.regs[indexReg].u64*array->elementSize), &core->constantPool.pool[offset], size);\
}

A_STOREF_CONST(8, 1)
A_STOREF_CONST(16, 2)
A_STOREF_CONST(32, 4)
A_STOREF_CONST(64, 8)
A_STOREF_CONST(ptr, PTR_SIZE)
#undef A_STOREF_CONST

void a_loadf(TypeV_Core* core){
    const uint8_t target = core->program.bytecode[core->registers.ip++];
    const uint8_t index = core->program.bytecode[core->registers.ip++];
    const uint8_t element_size = core->program.bytecode[core->registers.ip++];

    ASSERT(target < MAX_REG, "Invalid register index");
    ASSERT(index < MAX_REG, "Invalid register index");
    ASSERT(element_size <= 8, "Invalid byte size");

    TypeV_Array* array = (TypeV_Array*)core->registers.regs[19].ptr;
    memcpy(&core->registers.regs[target], array->data+(core->registers.regs[index].u64*array->elementSize), element_size);
}

void push(TypeV_Core* core){
    const uint8_t source = core->program.bytecode[core->registers.ip++];
    uint8_t bytesize = core->program.bytecode[core->registers.ip++];
    if(bytesize == 0) bytesize = PTR_SIZE;

    ASSERT(source < MAX_REG, "Invalid register index");
    ASSERT(bytesize <= 8, "Invalid byte size");

    switch(bytesize){
        case 0:
            stack_push_ptr(core, core->registers.regs[source].ptr);
            break;
        case 1:
            stack_push_8(core, core->registers.regs[source].u8);
            break;
        case 2:
            stack_push_16(core, core->registers.regs[source].u16);
            break;
        case 4:
            stack_push_32(core, core->registers.regs[source].u32);
            break;
        case 8:
            stack_push_64(core, core->registers.regs[source].u64);
            break;
        default:
            LOG_ERROR("Invalid byte size %d", bytesize);
            exit(-1);
    }
}

void push_const(TypeV_Core* core){
    const uint8_t offset_length = core->program.bytecode[core->registers.ip++];
    size_t offset = 0; /* we do not increment offset here*/
    memcpy(&offset, &core->program.bytecode[core->registers.ip],  offset_length);
    core->registers.ip += offset_length;
    ASSERT(offset < core->constantPool.length, "Invalid constant offset");

    // get bytes
    uint8_t bytesize = core->program.bytecode[core->registers.ip++];
    if(bytesize == 0) bytesize = PTR_SIZE;
    ASSERT(bytesize <= 8, "Invalid byte size");

    switch (bytesize) {
        case 0:
            stack_push_ptr(core, (size_t)&core->constantPool.pool[offset]);
            break;
        case 1:
            stack_push_8(core, core->constantPool.pool[offset]);
            break;
        case 2:
            stack_push_16(core, *((uint16_t*)&core->constantPool.pool[offset]));
            break;
        case 4:
            stack_push_32(core, *((uint32_t*)&core->constantPool.pool[offset]));
            break;
        case 8:
            stack_push_64(core, *((uint64_t*)&core->constantPool.pool[offset]));
            break;
        default:
            LOG_ERROR("Invalid byte size %d", bytesize);
            exit(-1);
    }
}

void pop(TypeV_Core* core){
    const uint8_t target = core->program.bytecode[core->registers.ip++];
    uint8_t bytesize = core->program.bytecode[core->registers.ip++];
    if(bytesize == 0) bytesize = PTR_SIZE;

    ASSERT(target < MAX_REG, "Invalid register index");
    ASSERT(bytesize <= 8, "Invalid byte size");

    switch(bytesize){
        case 0:
            stack_pop_ptr(core, &core->registers.regs[target].ptr);
            break;
        case 1:
            stack_pop_8(core, &core->registers.regs[target].u8);
            break;
        case 2:
            stack_pop_16(core, &core->registers.regs[target].u16);
            break;
        case 4:
            stack_pop_32(core, &core->registers.regs[target].u32);
            break;
        case 8:
            stack_pop_64(core, &core->registers.regs[target].u64);
            break;
        default:
            LOG_ERROR("Invalid byte size %d", bytesize);
            exit(-1);
    }
}

void frame_init_args(TypeV_Core* core){
    const uint8_t offset_length = core->program.bytecode[core->registers.ip++];
    size_t offset = 0; /* we do not increment offset here*/
    memcpy(&offset, &core->program.bytecode[core->registers.ip],  offset_length);
    core->registers.ip += offset_length;

    stack_frame_alloc_args(core, offset);
}

void frame_init_locals(TypeV_Core* core){
    const uint8_t offset_length = core->program.bytecode[core->registers.ip++];
    size_t offset = 0; /* we do not increment offset here*/
    memcpy(&offset, &core->program.bytecode[core->registers.ip],  offset_length);
    core->registers.ip += offset_length;

    stack_frame_alloc_locals(core, offset);
}

void frame_rm(TypeV_Core* core){
    stack_frame_remove(core);
}

void frame_precall(TypeV_Core* core){
    stack_frame_precall_push(core);
}

void fn_main(TypeV_Core* core){
    if(core->registers.fe != core->registers.sp){
        LOG_ERROR("Function call stack is not properly intialized, fp: %zu, fe: %zu, sp: %zu", core->registers.fp, core->registers.fe, core->registers.sp);
        exit(-1);
    }
}

void fn_ret(TypeV_Core* core){
    stack_frame_postcall_pop(core);
    LOG_INFO("Returning from function towards %p", (void*)core->registers.ip);
}

void fn_call(TypeV_Core* core){
    // get the register
    const uint8_t target = core->program.bytecode[core->registers.ip++];
    ASSERT(target < MAX_REG, "Invalid register index");

    const size_t adr = core->registers.regs[target].ptr;
    LOG_INFO("Calling function at %p", (void*)adr);
    // update the pushed ip to the current ip
    // fp - sizeof(Registers) -8 (for the unpushed R20) and -8 for the flags

    memcpy(core->stack.stack+core->registers.fp-(sizeof(TypeV_Registers) - 16), &core->registers.ip, sizeof(uint64_t));

    // jump to the address
    core->registers.ip = adr;
}

void fn_calli(TypeV_Core* core){
    // same as fn_call but takes immediate value
    const uint8_t offset_length = core->program.bytecode[core->registers.ip++];
    size_t offset = 0; /* we do not increment offset here*/
    memcpy(&offset, &core->program.bytecode[core->registers.ip],  offset_length);
    core->registers.ip += offset_length;

    LOG_INFO("Calling function at %p", (void*)offset);
    // update the pushed ip to the current ip
    // fp - sizeof(Registers) -8 (for the unpushed R20) and -8 for the flags
    memcpy(core->stack.stack+core->registers.fp-(sizeof(TypeV_Registers) - 16), &core->registers.ip, sizeof(uint64_t));

    // jump to the address
    core->registers.ip = offset;
}

#define OP_CAST(d1, d2, type) \
void cast_##d1##_##d2(TypeV_Core* core){ \
    uint8_t op1 = core->program.bytecode[core->registers.ip++];\
    core->registers.regs[op1].d2 = (type) core->registers.regs[op1].d1;\
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

#define OP_UPCAST(d1, d2, type) \
void upcast_##d1##_##d2(TypeV_Core* core){ \
    uint8_t op1 = core->program.bytecode[core->registers.ip++];\
    core->registers.regs[op1].d2 = (type) core->registers.regs[op1].d1;\
}

OP_UPCAST(i8, i16, int16_t)
OP_UPCAST(u8, u16, uint16_t)
OP_UPCAST(i16, i32, int32_t)
OP_UPCAST(u16, u32, uint32_t)
OP_UPCAST(i32, i64, int64_t)
OP_UPCAST(u32, u64, uint64_t)
OP_UPCAST(f32, f64, double)
#undef OP_UPCAST

#define OP_DOWNCAST(d1, d2, type) \
void dcast_##d1##_##d2(TypeV_Core* core){ \
    uint8_t op1 = core->program.bytecode[core->registers.ip++];\
    core->registers.regs[op1].d2 = (type) core->registers.regs[op1].d1;\
}

OP_DOWNCAST(i16, i8, int8_t)
OP_DOWNCAST(u16, u8, uint8_t)
OP_DOWNCAST(i32, i16, int16_t)
OP_DOWNCAST(u32, u16, uint16_t)
OP_DOWNCAST(i64, i32, int32_t)
OP_DOWNCAST(u64, u32, uint32_t)
OP_DOWNCAST(f64, f32, float)
#undef OP_DOWNCAST

#define OP_BINARY(name, type, op)\
void name##_##type(TypeV_Core* core){\
    uint8_t op1 = core->program.bytecode[core->registers.ip++];\
    uint8_t op2 = core->program.bytecode[core->registers.ip++];\
    uint8_t target = core->program.bytecode[core->registers.ip++];\
    core->registers.regs[target].type = core->registers.regs[op1].type op core->registers.regs[op2].type;\
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
    uint8_t op1 = core->program.bytecode[core->registers.ip++];
    uint8_t op2 = core->program.bytecode[core->registers.ip++];
    uint8_t target = core->program.bytecode[core->registers.ip++];
    core->registers.regs[target].ptr = core->registers.regs[op1].ptr + core->registers.regs[op2].u8;
}

void add_ptr_u16(TypeV_Core* core){
    uint8_t op1 = core->program.bytecode[core->registers.ip++];
    uint8_t op2 = core->program.bytecode[core->registers.ip++];
    uint8_t target = core->program.bytecode[core->registers.ip++];
    core->registers.regs[target].ptr = core->registers.regs[op1].ptr + core->registers.regs[op2].u16;
}

void add_ptr_u32(TypeV_Core* core){
    uint8_t op1 = core->program.bytecode[core->registers.ip++];
    uint8_t op2 = core->program.bytecode[core->registers.ip++];
    uint8_t target = core->program.bytecode[core->registers.ip++];
    core->registers.regs[target].ptr = core->registers.regs[op1].ptr + core->registers.regs[op2].u32;
}

void add_ptr_u64(TypeV_Core* core){
    uint8_t op1 = core->program.bytecode[core->registers.ip++];
    uint8_t op2 = core->program.bytecode[core->registers.ip++];
    uint8_t target = core->program.bytecode[core->registers.ip++];
    core->registers.regs[target].ptr = core->registers.regs[op1].ptr + core->registers.regs[op2].u64;
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
    uint8_t op1 = core->program.bytecode[core->registers.ip++];
    uint8_t op2 = core->program.bytecode[core->registers.ip++];
    uint8_t target = core->program.bytecode[core->registers.ip++];
    core->registers.regs[target].ptr = core->registers.regs[op1].ptr - core->registers.regs[op2].u8;
}

void sub_ptr_u16(TypeV_Core* core){
    uint8_t op1 = core->program.bytecode[core->registers.ip++];
    uint8_t op2 = core->program.bytecode[core->registers.ip++];
    uint8_t target = core->program.bytecode[core->registers.ip++];
    core->registers.regs[target].ptr = core->registers.regs[op1].ptr - core->registers.regs[op2].u16;
}

void sub_ptr_u32(TypeV_Core* core){
    uint8_t op1 = core->program.bytecode[core->registers.ip++];
    uint8_t op2 = core->program.bytecode[core->registers.ip++];
    uint8_t target = core->program.bytecode[core->registers.ip++];
    core->registers.regs[target].ptr = core->registers.regs[op1].ptr - core->registers.regs[op2].u32;
}

void sub_ptr_u64(TypeV_Core* core){
    uint8_t op1 = core->program.bytecode[core->registers.ip++];
    uint8_t op2 = core->program.bytecode[core->registers.ip++];
    uint8_t target = core->program.bytecode[core->registers.ip++];
    core->registers.regs[target].ptr = core->registers.regs[op1].ptr - core->registers.regs[op2].u64;
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
    uint8_t op1 = core->program.bytecode[core->registers.ip++];
    uint8_t op2 = core->program.bytecode[core->registers.ip++];

    // Get both values as int8_t
    int8_t v1 = core->registers.regs[op1].i8;
    int8_t v2 = core->registers.regs[op2].i8;

    // Perform the comparison as int16_t
    int16_t result = (int16_t)(int)((int16_t)v1 - (int16_t)v2);

    // Set Zero Flag (ZF)
    WRITE_FLAG(core->registers.flags, FLAG_ZF, result==0);

    // Set Sign Flag (SF) using bit shifting
    WRITE_FLAG(core->registers.flags, FLAG_SF, (result >> 15) & 1);

    // Set Overflow Flag (OF) using bit shifting
    int v1_sign = (v1 >> 7) & 1;
    int v2_sign = (v2 >> 7) & 1;
    int result_sign = (result >> 15) & 1;
    WRITE_FLAG(core->registers.flags, FLAG_OF, (v1_sign != v2_sign) && (v1_sign != result_sign));

    // Set SignType Flag (STF) for signed operation
    WRITE_FLAG(core->registers.flags, FLAG_TF, 1); // Corrected to FLAG_STF

    // Clear Carry Flag (CF) as it is not relevant here
    FLAG_CLEAR(core->registers.flags, FLAG_CF);
}

void cmp_u8(TypeV_Core* core) {
    // Get first and second registers
    uint8_t op1 = core->program.bytecode[core->registers.ip++];
    uint8_t op2 = core->program.bytecode[core->registers.ip++];

    // Get both values as uint8_t
    uint8_t v1 = core->registers.regs[op1].u8;
    uint8_t v2 = core->registers.regs[op2].u8;

    // Perform the comparison as uint16_t to handle any potential overflow
    uint16_t result = (uint16_t)v1 - (uint16_t)v2;

    // Set Zero Flag (ZF) using bitwise operations
    WRITE_FLAG(core->registers.flags, FLAG_ZF, result==0);

    // Overflow Flag (OF) and Sign Flag (SF) are not used in unsigned comparisons
    // Clearing OF and SF for clarity
    WRITE_FLAG(core->registers.flags, FLAG_OF, 0);
    WRITE_FLAG(core->registers.flags, FLAG_SF, 0);

    // Set Carry Flag (CF) if there was a borrow in the subtraction
    WRITE_FLAG(core->registers.flags, FLAG_CF, v1 < v2);

    // Set SignType Flag (STF) for unsigned operation
    WRITE_FLAG(core->registers.flags, FLAG_TF, 0);
}

void cmp_i16(TypeV_Core* core) {
    // Get first and second registers
    uint8_t op1 = core->program.bytecode[core->registers.ip++];
    uint8_t op2 = core->program.bytecode[core->registers.ip++];

    // Get both values as int16_t
    int16_t v1 = core->registers.regs[op1].i16;
    int16_t v2 = core->registers.regs[op2].i16;

    // Perform the comparison as int32_t to handle overflow properly
    int32_t result = (int32_t)v1 - (int32_t)v2;

    // Set Zero Flag (ZF) using bitwise operations
    WRITE_FLAG(core->registers.flags, FLAG_ZF, result==0);

    // Set Sign Flag (SF) using bit shifting
    WRITE_FLAG(core->registers.flags, FLAG_SF, (result >> 31) & 1);

    // Set Overflow Flag (OF) using bit shifting
    int v1_sign = (v1 >> 15) & 1;
    int v2_sign = (v2 >> 15) & 1;
    int result_sign = (result >> 31) & 1;
    WRITE_FLAG(core->registers.flags, FLAG_OF, (v1_sign != v2_sign) && (v1_sign != result_sign));

    // Set SignType Flag (STF) for signed operation
    WRITE_FLAG(core->registers.flags, FLAG_TF, 1);
}

void cmp_u16(TypeV_Core* core) {
    // Get first and second registers
    uint8_t op1 = core->program.bytecode[core->registers.ip++];
    uint8_t op2 = core->program.bytecode[core->registers.ip++];

    // Get both values as uint16_t
    uint16_t v1 = core->registers.regs[op1].u16;
    uint16_t v2 = core->registers.regs[op2].u16;

    // Perform the comparison as uint32_t to handle any potential wrap-around or borrow
    uint32_t result = (uint32_t)v1 - (uint32_t)v2;

    // Set Zero Flag (ZF) using bitwise operations
    WRITE_FLAG(core->registers.flags, FLAG_ZF, result==0);

    // Overflow Flag (OF) and Sign Flag (SF) are not used in unsigned comparisons
    // Clearing OF and SF for clarity
    WRITE_FLAG(core->registers.flags, FLAG_OF, 0);
    WRITE_FLAG(core->registers.flags, FLAG_SF, 0);

    // Set Carry Flag (CF) if there was a borrow in the subtraction
    WRITE_FLAG(core->registers.flags, FLAG_CF, v1 < v2);

    // Set SignType Flag (STF) for unsigned operation
    WRITE_FLAG(core->registers.flags, FLAG_TF, 0);
}

void cmp_i32(TypeV_Core* core) {
    // Get first and second registers
    uint8_t op1 = core->program.bytecode[core->registers.ip++];
    uint8_t op2 = core->program.bytecode[core->registers.ip++];

    // Get both values as int32_t
    int32_t v1 = core->registers.regs[op1].i32;
    int32_t v2 = core->registers.regs[op2].i32;

    // Perform the comparison as int64_t to handle overflow properly
    int64_t result = (int64_t)v1 - (int64_t)v2;

    // Set Zero Flag (ZF) using bitwise operations
    WRITE_FLAG(core->registers.flags, FLAG_ZF, result==0);

    // Set Sign Flag (SF) using bit shifting
    WRITE_FLAG(core->registers.flags, FLAG_SF, (result >> 63) & 1);

    // Set Overflow Flag (OF) using bit shifting
    int v1_sign = (v1 >> 31) & 1;
    int v2_sign = (v2 >> 31) & 1;
    int64_t result_sign = (result >> 63) & 1;
    WRITE_FLAG(core->registers.flags, FLAG_OF, (v1_sign != v2_sign) && (v1_sign != result_sign));

    // Set SignType Flag (STF) for signed operation
    WRITE_FLAG(core->registers.flags, FLAG_TF, 1);
}

void cmp_u32(TypeV_Core* core) {
    // Get first and second registers
    uint8_t op1 = core->program.bytecode[core->registers.ip++];
    uint8_t op2 = core->program.bytecode[core->registers.ip++];

    // Get both values as uint32_t
    uint32_t v1 = core->registers.regs[op1].u32;
    uint32_t v2 = core->registers.regs[op2].u32;

    // Perform the comparison as uint64_t to handle wrap-around properly
    uint64_t result = (uint64_t)v1 - (uint64_t)v2;

    // Set Zero Flag (ZF) using bitwise operations
    WRITE_FLAG(core->registers.flags, FLAG_ZF, result==0);

    // Overflow Flag (OF) and Sign Flag (SF) are not used in unsigned comparisons
    // Clearing OF and SF for clarity
    WRITE_FLAG(core->registers.flags, FLAG_OF, 0);
    WRITE_FLAG(core->registers.flags, FLAG_SF, 0);

    // Set Carry Flag (CF) if there was a borrow in the subtraction
    WRITE_FLAG(core->registers.flags, FLAG_CF, v1 < v2);

    // Set SignType Flag (STF) for unsigned operation
    WRITE_FLAG(core->registers.flags, FLAG_TF, 0);
}

void cmp_i64(TypeV_Core* core) {
    // Get first and second registers
    uint8_t op1 = core->program.bytecode[core->registers.ip++];
    uint8_t op2 = core->program.bytecode[core->registers.ip++];

    // Get both values as int64_t
    int64_t v1 = core->registers.regs[op1].i64;
    int64_t v2 = core->registers.regs[op2].i64;

    // Detecting overflow in 64-bit integer subtraction can be complex
    // Overflow occurs if the sign of v1 is different from v2 AND
    // the sign of the result (v1 - v2) is different from v1

    // Direct subtraction for Zero and Sign flags
    int64_t result = v1 - v2;

    // Set Zero Flag (ZF)
    WRITE_FLAG(core->registers.flags, FLAG_ZF, result == 0);

    // Set Sign Flag (SF)
    WRITE_FLAG(core->registers.flags, FLAG_SF, result < 0);

    // Set Overflow Flag (OF)
    int v1_sign = v1 < 0;
    int v2_sign = v2 < 0;
    int result_sign = result < 0;
    WRITE_FLAG(core->registers.flags, FLAG_OF, (v1_sign != v2_sign) && (v1_sign != result_sign));

    // Set SignType Flag (STF) for signed operation
    WRITE_FLAG(core->registers.flags, FLAG_TF, 1);
}

void cmp_u64(TypeV_Core* core) {
    // Get first and second registers
    uint8_t op1 = core->program.bytecode[core->registers.ip++];
    uint8_t op2 = core->program.bytecode[core->registers.ip++];

    // Get both values as uint64_t
    uint64_t v1 = core->registers.regs[op1].u64;
    uint64_t v2 = core->registers.regs[op2].u64;

    // Perform the subtraction
    uint64_t result = v1 - v2;

    // Set Zero Flag (ZF)
    WRITE_FLAG(core->registers.flags, FLAG_ZF, result == 0);

    // Overflow Flag (OF) and Sign Flag (SF) are not relevant in unsigned comparisons
    // Clearing them for clarity
    WRITE_FLAG(core->registers.flags, FLAG_OF, 0);
    WRITE_FLAG(core->registers.flags, FLAG_SF, 0);

    // Set Carry Flag (CF) if there was a borrow in the subtraction
    WRITE_FLAG(core->registers.flags, FLAG_CF, v1 < v2);

    // Set SignType Flag (STF) for unsigned operation
    WRITE_FLAG(core->registers.flags, FLAG_TF, 0);
}

void cmp_f32(TypeV_Core* core) {
    // Get first and second registers
    uint8_t op1 = core->program.bytecode[core->registers.ip++];
    uint8_t op2 = core->program.bytecode[core->registers.ip++];

    // Get both values as float
    float v1 = core->registers.regs[op1].f32;
    float v2 = core->registers.regs[op2].f32;

    // Compare the float values
    float result = v1 - v2;

    // Set Zero Flag (ZF)
    WRITE_FLAG(core->registers.flags, FLAG_ZF, result == 0.0f);

    // Set Sign Flag (SF) - Negative if result is negative, clear otherwise
    WRITE_FLAG(core->registers.flags, FLAG_SF, result < 0.0f);

    // For floating-point comparison, Overflow Flag (OF) and Carry Flag (CF) are not applicable
    WRITE_FLAG(core->registers.flags, FLAG_OF, 0);
    WRITE_FLAG(core->registers.flags, FLAG_CF, 0);

    // Set SignType Flag (STF) for floating-point operation
    WRITE_FLAG(core->registers.flags, FLAG_TF, 1);
}

void cmp_f64(TypeV_Core* core) {
    // Get first and second registers
    uint8_t op1 = core->program.bytecode[core->registers.ip++];
    uint8_t op2 = core->program.bytecode[core->registers.ip++];

    // Get both values as double
    double v1 = core->registers.regs[op1].f64;
    double v2 = core->registers.regs[op2].f64;

    // Compare the double values
    double result = v1 - v2;

    // Set Zero Flag (ZF)
    WRITE_FLAG(core->registers.flags, FLAG_ZF, result == 0.0);

    // Set Sign Flag (SF) - Negative if result is negative, clear otherwise
    WRITE_FLAG(core->registers.flags, FLAG_SF, result < 0.0);

    // For floating-point comparison, Overflow Flag (OF) and Carry Flag (CF) are not applicable
    WRITE_FLAG(core->registers.flags, FLAG_OF, 0);
    WRITE_FLAG(core->registers.flags, FLAG_CF, 0);

    // Set SignType Flag (STF) for floating-point operation
    WRITE_FLAG(core->registers.flags, FLAG_TF, 1);
}

void cmp_ptr(TypeV_Core* core) {
    // Get first and second registers
    uint8_t op1 = core->program.bytecode[core->registers.ip++];
    uint8_t op2 = core->program.bytecode[core->registers.ip++];

    // Get both values as pointers (assuming 64-bit pointers)
    size_t ptr1 = core->registers.regs[op1].ptr;
    size_t ptr2 = core->registers.regs[op2].ptr;

    // Perform the comparison as uint64_t
    size_t result = ptr1 - ptr2;

    // Set Zero Flag (ZF)
    WRITE_FLAG(core->registers.flags, FLAG_ZF, result == 0);

    // Overflow Flag (OF) and Sign Flag (SF) are not relevant in pointer comparisons
    WRITE_FLAG(core->registers.flags, FLAG_OF, 0);
    WRITE_FLAG(core->registers.flags, FLAG_SF, 0);

    // Set Carry Flag (CF) if there was a "borrow" in the subtraction
    WRITE_FLAG(core->registers.flags, FLAG_CF, ptr1 < ptr2);

    // Set SignType Flag (STF) for unsigned operation
    WRITE_FLAG(core->registers.flags, FLAG_TF, 0);
}

void band_8(TypeV_Core* core){
    uint8_t op1 = core->program.bytecode[core->registers.ip++];
    uint8_t op2 = core->program.bytecode[core->registers.ip++];
    uint8_t target = core->program.bytecode[core->registers.ip++];

    core->registers.regs[target].u8 = core->registers.regs[op1].u8 & core->registers.regs[op2].u8;
}

void band_16(TypeV_Core* core){
    uint8_t op1 = core->program.bytecode[core->registers.ip++];
    uint8_t op2 = core->program.bytecode[core->registers.ip++];
    uint8_t target = core->program.bytecode[core->registers.ip++];

    core->registers.regs[target].u16 = core->registers.regs[op1].u16 & core->registers.regs[op2].u16;
}

void band_32(TypeV_Core* core){
    uint8_t op1 = core->program.bytecode[core->registers.ip++];
    uint8_t op2 = core->program.bytecode[core->registers.ip++];
    uint8_t target = core->program.bytecode[core->registers.ip++];

    core->registers.regs[target].u32 = core->registers.regs[op1].u32 & core->registers.regs[op2].u32;
}

void band_64(TypeV_Core* core){
    uint8_t op1 = core->program.bytecode[core->registers.ip++];
    uint8_t op2 = core->program.bytecode[core->registers.ip++];
    uint8_t target = core->program.bytecode[core->registers.ip++];

    core->registers.regs[target].u64 = core->registers.regs[op1].u64 & core->registers.regs[op2].u64;
}

void bor_8(TypeV_Core* core){
    uint8_t op1 = core->program.bytecode[core->registers.ip++];
    uint8_t op2 = core->program.bytecode[core->registers.ip++];
    uint8_t target = core->program.bytecode[core->registers.ip++];

    core->registers.regs[target].u8 = core->registers.regs[op1].u8 | core->registers.regs[op2].u8;
}

void bor_16(TypeV_Core* core){
    uint8_t op1 = core->program.bytecode[core->registers.ip++];
    uint8_t op2 = core->program.bytecode[core->registers.ip++];
    uint8_t target = core->program.bytecode[core->registers.ip++];

    core->registers.regs[target].u16 = core->registers.regs[op1].u16 | core->registers.regs[op2].u16;
}

void bor_32(TypeV_Core* core){
    uint8_t op1 = core->program.bytecode[core->registers.ip++];
    uint8_t op2 = core->program.bytecode[core->registers.ip++];
    uint8_t target = core->program.bytecode[core->registers.ip++];

    core->registers.regs[target].u32 = core->registers.regs[op1].u32 | core->registers.regs[op2].u32;
}

void bor_64(TypeV_Core* core){
    uint8_t op1 = core->program.bytecode[core->registers.ip++];
    uint8_t op2 = core->program.bytecode[core->registers.ip++];
    uint8_t target = core->program.bytecode[core->registers.ip++];

    core->registers.regs[target].u64 = core->registers.regs[op1].u64 | core->registers.regs[op2].u64;
}

void bxor_8(TypeV_Core* core){
    uint8_t op1 = core->program.bytecode[core->registers.ip++];
    uint8_t target = core->program.bytecode[core->registers.ip++];

    core->registers.regs[target].u8 = core->registers.regs[op1].u8 ^ core->registers.regs[target].u8;
}

void bxor_16(TypeV_Core* core){
    uint8_t op1 = core->program.bytecode[core->registers.ip++];
    uint8_t target = core->program.bytecode[core->registers.ip++];

    core->registers.regs[target].u16 = core->registers.regs[op1].u16 ^ core->registers.regs[target].u16;
}

void bxor_32(TypeV_Core* core){
    uint8_t op1 = core->program.bytecode[core->registers.ip++];
    uint8_t target = core->program.bytecode[core->registers.ip++];

    core->registers.regs[target].u32 = core->registers.regs[op1].u32 ^ core->registers.regs[target].u32;
}

void bxor_64(TypeV_Core* core){
    uint8_t op1 = core->program.bytecode[core->registers.ip++];
    uint8_t target = core->program.bytecode[core->registers.ip++];

    core->registers.regs[target].u64 = core->registers.regs[op1].u64 ^ core->registers.regs[target].u64;
}

void bnot_8(TypeV_Core* core){
    uint8_t target = core->program.bytecode[core->registers.ip++];

    core->registers.regs[target].u8 = ~core->registers.regs[target].u8;
}

void bnot_16(TypeV_Core* core){
    uint8_t target = core->program.bytecode[core->registers.ip++];

    core->registers.regs[target].u16 = ~core->registers.regs[target].u16;
}

void bnot_32(TypeV_Core* core){
    uint8_t target = core->program.bytecode[core->registers.ip++];

    core->registers.regs[target].u32 = ~core->registers.regs[target].u32;
}

void bnot_64(TypeV_Core* core){
    uint8_t target = core->program.bytecode[core->registers.ip++];

    core->registers.regs[target].u64 = ~core->registers.regs[target].u64;
}

void and(TypeV_Core* core){
    uint8_t op1 = core->program.bytecode[core->registers.ip++];
    uint8_t target = core->program.bytecode[core->registers.ip++];

    core->registers.regs[target].u8 = core->registers.regs[op1].u8 && core->registers.regs[target].u8;
}

void or(TypeV_Core* core){
    uint8_t op1 = core->program.bytecode[core->registers.ip++];
    uint8_t target = core->program.bytecode[core->registers.ip++];

    core->registers.regs[target].u8 = core->registers.regs[op1].u8 || core->registers.regs[target].u8;
}

void not(TypeV_Core* core){
    uint8_t target = core->program.bytecode[core->registers.ip++];

    core->registers.regs[target].u8 = !core->registers.regs[target].u8;
}

void jmp(TypeV_Core* core){
    const uint8_t offset_length = core->program.bytecode[core->registers.ip++];
    size_t offset = 0; /* we do not increment offset here*/
    memcpy(&offset, &core->program.bytecode[core->registers.ip],  offset_length);
    core->registers.ip = offset;
}

void jmp_e(TypeV_Core* core){
    const uint8_t offset_length = core->program.bytecode[core->registers.ip++];
    size_t offset = 0; /* we do not increment offset here*/


    if(FLAG_CHECK(core->registers.flags, FLAG_ZF)){
        memcpy(&offset, &core->program.bytecode[core->registers.ip],  offset_length);
        core->registers.ip = offset;
    }
    else {
        core->registers.ip += offset_length;
    }
}

void jmp_ne(TypeV_Core* core){
    const uint8_t offset_length = core->program.bytecode[core->registers.ip++];
    size_t offset = 0; /* we do not increment offset here*/

    if(!FLAG_CHECK(core->registers.flags, FLAG_ZF)){
        memcpy(&offset, &core->program.bytecode[core->registers.ip],  offset_length);
        core->registers.ip = offset;
    }
    else {
        core->registers.ip += offset_length;
    }
}

void jmp_g(TypeV_Core* core){
    const uint8_t offset_length = core->program.bytecode[core->registers.ip++];
    size_t offset = 0; /* we do not increment offset here*/

    uint8_t can_jump = 0;
    // Check if the last operation was signed or unsigned
    if (FLAG_CHECK(core->registers.flags, FLAG_TF)) {
        // Signed comparison
        // Jump if ZF is clear, and either SF equals OF, or OF is clear and SF is clear
        can_jump = !FLAG_CHECK(core->registers.flags, FLAG_ZF) &&
                        ((FLAG_CHECK(core->registers.flags, FLAG_SF) == FLAG_CHECK(core->registers.flags, FLAG_OF)) ||
                         (!FLAG_CHECK(core->registers.flags, FLAG_OF) && !FLAG_CHECK(core->registers.flags, FLAG_SF)));
    } else {
        // Unsigned comparison
        // Jump if ZF is clear and CF is clear (no borrow occurred)
        can_jump = !FLAG_CHECK(core->registers.flags, FLAG_ZF) &&
                        !FLAG_CHECK(core->registers.flags, FLAG_CF);
    }

    if(can_jump){
        memcpy(&offset, &core->program.bytecode[core->registers.ip],  offset_length);
        core->registers.ip = offset;
    }
    else {
        core->registers.ip += offset_length;
    }
}

void jmp_ge(TypeV_Core* core){
    const uint8_t offset_length = core->program.bytecode[core->registers.ip++];
    size_t offset = 0; /* we do not increment offset here*/

    uint8_t can_jump = 0;
    if (FLAG_CHECK(core->registers.flags, FLAG_TF)) {
        // Signed comparison
        // Jump if either SF equals OF, or OF is clear and SF is clear
        can_jump = (FLAG_CHECK(core->registers.flags, FLAG_SF) == FLAG_CHECK(core->registers.flags, FLAG_OF)) ||
                        (!FLAG_CHECK(core->registers.flags, FLAG_OF) && !FLAG_CHECK(core->registers.flags, FLAG_SF));
    } else {
        // Unsigned comparison
        // Jump if CF is clear (no borrow occurred)
        can_jump = !FLAG_CHECK(core->registers.flags, FLAG_CF);
    }

    if (can_jump) {
        // Perform the jump
        core->registers.ip = can_jump;
    }

    if(can_jump){
        memcpy(&offset, &core->program.bytecode[core->registers.ip],  offset_length);
        core->registers.ip = offset;
    }
    else {
        core->registers.ip += offset_length;
    }
}

void jmp_l(TypeV_Core* core){
    const uint8_t offset_length = core->program.bytecode[core->registers.ip++];
    size_t offset = 0; /* we do not increment offset here*/

    uint8_t can_jump = 0;
    if (FLAG_CHECK(core->registers.flags, FLAG_TF)) {
        // Signed comparison
        // Jump if SF does not equal OF
        can_jump = FLAG_CHECK(core->registers.flags, FLAG_SF) != FLAG_CHECK(core->registers.flags, FLAG_OF);
    } else {
        // Unsigned comparison
        // Jump if CF is set (borrow occurred)
        can_jump = FLAG_CHECK(core->registers.flags, FLAG_CF);
    }

    if(can_jump){
        memcpy(&offset, &core->program.bytecode[core->registers.ip],  offset_length);
        core->registers.ip = offset;
    }
    else {
        core->registers.ip += offset_length;
    }
}

void jmp_le(TypeV_Core* core){
    const uint8_t offset_length = core->program.bytecode[core->registers.ip++];
    size_t offset = 0; /* we do not increment offset here*/


    uint8_t can_jump = 0;
    if (FLAG_CHECK(core->registers.flags, FLAG_TF)) {
        // Signed comparison
        // Jump if ZF is set, or SF does not equal OF
        can_jump = FLAG_CHECK(core->registers.flags, FLAG_ZF) ||
                        (FLAG_CHECK(core->registers.flags, FLAG_SF) != FLAG_CHECK(core->registers.flags, FLAG_OF));
    } else {
        // Unsigned comparison
        // Jump if CF is set (borrow occurred), or ZF is set
        can_jump = FLAG_CHECK(core->registers.flags, FLAG_CF) ||
                        FLAG_CHECK(core->registers.flags, FLAG_ZF);
    }

    if(can_jump){
        memcpy(&offset, &core->program.bytecode[core->registers.ip],  offset_length);
        core->registers.ip = offset;
    }
    else {
        core->registers.ip += offset_length;
    }
}

void ld_ffi(TypeV_Core* core){
    uint8_t dest = core->program.bytecode[core->registers.ip++];
    uint8_t offset_length = core->program.bytecode[core->registers.ip++];
    size_t offset = 0;
    memcpy(&offset, &core->program.bytecode[core->registers.ip], offset_length);
    core->registers.ip += offset_length;
    size_t namePtr = (size_t)(core->constantPool.pool + offset);
    core->registers.regs[dest].ptr = core_ffi_load(core, namePtr);
}

void call_ffi(TypeV_Core* core){
    uint8_t reg = core->program.bytecode[core->registers.ip++];
    uint8_t index_length = core->program.bytecode[core->registers.ip++];
    size_t index = 0;
    memcpy(&index, &core->program.bytecode[core->registers.ip], index_length);

    core->registers.ip += index_length;

    TypeV_FFI * module = (TypeV_FFI *)core->registers.regs[reg].ptr;

    TypeV_FFIFunc ffi_fn = module->functions[index];
    ffi_fn(core);
}

void close_ffi(TypeV_Core* core){
    uint8_t reg = core->program.bytecode[core->registers.ip++];
    core_ffi_close(core, core->registers.regs[reg].ptr);
}

void p_alloc(TypeV_Core* core){
    uint8_t reg = core->program.bytecode[core->registers.ip++];
    uint8_t size_length = core->program.bytecode[core->registers.ip++];
    size_t size = 0;
    memcpy(&size, &core->program.bytecode[core->registers.ip], size_length);
    core->registers.ip += size_length;

    core->registers.regs[reg].ptr = (size_t)engine_spawnCore(core->engineRef, core, size);
}

void p_dequeue(TypeV_Core* core){
    LOG_INFO("Core[%d] dequeueing 1/%d", core->id, core->messageInputQueue.length);
    uint8_t dest = core->program.bytecode[core->registers.ip++];
    // get the next queue element from core queue
    // check queue length
    if(core->messageInputQueue.length == 0) {
        LOG_ERROR("Core[%d] tried to dequeue from empty queue", core->id);
        ASSERT(0, "fail");
    }
    TypeV_IOMessage* msg = queue_dequeue(&core->messageInputQueue);
    core->registers.regs[dest].ptr = (size_t)msg->message;
}

void p_emit(TypeV_Core* core){
    uint8_t targetProcessReg = core->program.bytecode[core->registers.ip++];
    uint8_t dataReg = core->program.bytecode[core->registers.ip++];

    ASSERT(targetProcessReg < MAX_REG, "Invalid register index");
    ASSERT(dataReg < MAX_REG, "Invalid register index");

    size_t data_ptr = core->registers.regs[dataReg].ptr;
    TypeV_Core* target = (TypeV_Core*)core->registers.regs[targetProcessReg].ptr;

    LOG_INFO("Core[%d] emitting message to Core[%d]", core->id, target->id);

    TypeV_IOMessage* msg = malloc(sizeof(TypeV_IOMessage));
    msg->sender = core->id;
    msg->message = (void*)data_ptr;

    core_enqueue_message(target, msg);
}

void p_wait_queue(TypeV_Core* core){
    LOG_INFO("Core[%d] waiting for queue", core->id);
    if(core->messageInputQueue.length == 0){
        core_queue_await(core);
    }
}

void p_queue_size(TypeV_Core* core){
    uint8_t dest = core->program.bytecode[core->registers.ip++];
    core->registers.regs[dest].u64 = core->messageInputQueue.length;
}

void p_send_sig(TypeV_Core * core){
    uint8_t targetProcessReg = core->program.bytecode[core->registers.ip++];
    uint8_t sig = core->program.bytecode[core->registers.ip++];

    ASSERT(targetProcessReg < MAX_REG, "Invalid register index");
    ASSERT(sig < CSIG_KILL, "Invalid signal value");

    TypeV_Core* target = (TypeV_Core*)core->registers.regs[targetProcessReg].ptr;

    LOG_INFO("Core[%d] sending signal %d to Core[%d]", core->id, sig, target->id);

    core_recieve_signal(target, sig);
}

void p_id(TypeV_Core* core){
    uint8_t dest = core->program.bytecode[core->registers.ip++];
    uint8_t processReg = core->program.bytecode[core->registers.ip++];
    ASSERT(dest < MAX_REG, "Invalid register index");
    ASSERT(processReg < MAX_REG, "Invalid register index");

    TypeV_Core* c = (TypeV_Core*)core->registers.regs[processReg].ptr;

    core->registers.regs[dest].u32 = c->id;
}

void p_cid(TypeV_Core* core){
    uint8_t dest = core->program.bytecode[core->registers.ip++];
    ASSERT(dest < MAX_REG, "Invalid register index");
    core->registers.regs[dest].u32 = core->id;
}

void p_state(TypeV_Core* core){
    uint8_t dest = core->program.bytecode[core->registers.ip++];
    uint8_t processReg = core->program.bytecode[core->registers.ip++];
    ASSERT(dest < MAX_REG, "Invalid register index");
    ASSERT(processReg < MAX_REG, "Invalid register index");

    TypeV_Core* c = (TypeV_Core*)core->registers.regs[processReg].ptr;

    core->registers.regs[dest].u8 = c->state;
}

void debug_reg(TypeV_Core* core){
    // read register index
    uint8_t i = core->program.bytecode[core->registers.ip++];
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
              core->registers.regs[i].i8,
              core->registers.regs[i].u8,
              core->registers.regs[i].i16,
              core->registers.regs[i].u16,
              core->registers.regs[i].i32,
              core->registers.regs[i].u32,
              core->registers.regs[i].i64,
              core->registers.regs[i].u64,
              core->registers.regs[i].f32,
              core->registers.regs[i].f64,
              (void*)core->registers.regs[i].ptr,
              core->registers.regs[i].u64);
    table_print(&t, 2000, stdout);
    table_free(&t);
}

void halt(TypeV_Core* core) {
    core->isRunning = 0;
    core->state = CS_TERMINATED;
    engine_detach_core(core->engineRef, core);
}

void vm_health(TypeV_Core* core){
    uint8_t dest = core->program.bytecode[core->registers.ip++];
    ASSERT(dest < MAX_REG, "Invalid register index");

    core->registers.regs[dest].u8 = core->engineRef->health;
}


