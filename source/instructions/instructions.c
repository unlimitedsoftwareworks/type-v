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
    const uint8_t source = core->program.bytecode[core->registers.ip++];\
    const uint8_t offset_length = core->program.bytecode[core->registers.ip++];\
    size_t offset = 0; /* we do not increment offset here*/\
    memcpy(&offset, &core->program.bytecode[core->registers.ip],  offset_length);\
    core->registers.ip += offset_length;\
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
    const uint8_t source = core->program.bytecode[core->registers.ip++];\
    core->registers.ip += offset_length;\
    ASSERT(source < MAX_REG, "Invalid register index");\
    ASSERT(offset < core->globalPool.length, "Invalid global offset");         \
    memcpy(&core->globalPool.pool[offset], &core->registers.regs[source], size);\
}

MV_GLOBAL_REG(8, 1)
MV_GLOBAL_REG(16, 2)
MV_GLOBAL_REG(32, 4)
MV_GLOBAL_REG(64, 8)
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
MV_REG_GLOBAL(64, 8)
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
    size_t mem = core_alloc_struct(core, fields_count, struct_size);
    // move the pointer to R16
    core->registers.regs[16].ptr = mem;
}

void s_alloc_shadow(TypeV_Core* core){
    const uint8_t fields_count = core->program.bytecode[core->registers.ip++];

    size_t original_mem = core->registers.regs[16].ptr;

    // allocate memory for struct
    size_t mem = core_alloc_struct_shadow(core, fields_count, original_mem);
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

#define S_LOADF(bits, size) \
void s_loadf_##bits(TypeV_Core* core){\
    const uint8_t target = core->program.bytecode[core->registers.ip++];\
    const uint8_t field_index = core->program.bytecode[core->registers.ip++];\
    ASSERT(target < MAX_REG, "Invalid register index");\
    TypeV_Struct* struct_ptr = (TypeV_Struct*)core->registers.regs[16].ptr;\
    memcpy(&core->registers.regs[target], struct_ptr->data+struct_ptr->fieldOffsets[field_index], size);\
}

S_LOADF(8, 1)
S_LOADF(16, 2)
S_LOADF(32, 4)
S_LOADF(64, 8)
S_LOADF(ptr, PTR_SIZE)
#undef S_LOADF

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
    size_t mem = core_alloc_class_fields(core, fields_count, struct_size);
    // move the pointer to R17
    core->registers.regs[17].ptr = mem;
}

void c_allocm(TypeV_Core* core){
    const uint8_t methods_count = core->program.bytecode[core->registers.ip++];
    // allocate memory for struct
    TypeV_Class* c = (TypeV_Class*)core->registers.regs[17].ptr;
    core_alloc_class_methods(core, methods_count, c);
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

#define C_STOREF(bits, size) \
void c_storef_##bits(TypeV_Core* core){\
    const uint8_t field_index = core->program.bytecode[core->registers.ip++];\
    const uint8_t source = core->program.bytecode[core->registers.ip++];     \
    uint8_t bytesize = core->program.bytecode[core->registers.ip++];         \
    if(bytesize == 0) bytesize = PTR_SIZE;                                   \
    ASSERT(source < MAX_REG, "Invalid register index");                      \
    ASSERT(bytesize <= 8, "Invalid byte size");                              \
    TypeV_Class* c = (TypeV_Class*)core->registers.regs[17].ptr;             \
    memcpy(c->data+c->fieldsOffset[field_index], &core->registers.regs[source], bytesize); \
}

C_STOREF(8, 1)
C_STOREF(16, 2)
C_STOREF(32, 4)
C_STOREF(64, 8)
C_STOREF(ptr, PTR_SIZE)
#undef C_STOREF

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

#define C_LOADF(bits, size) \
void c_loadf_##bits(TypeV_Core* core){\
    const uint8_t target = core->program.bytecode[core->registers.ip++];\
    const uint8_t field_index = core->program.bytecode[core->registers.ip++];\
    ASSERT(target < MAX_REG, "Invalid register index");\
    TypeV_Class* c = (TypeV_Class*)core->registers.regs[17].ptr;             \
    memcpy(&core->registers.regs[target], c->data+c->fieldsOffset[field_index], size);\
}

C_LOADF(8, 1)
C_LOADF(16, 2)
C_LOADF(32, 4)
C_LOADF(64, 8)
C_LOADF(ptr, PTR_SIZE)
#undef C_LOADF

void i_alloc(TypeV_Core* core){
    const uint8_t fields_count = core->program.bytecode[core->registers.ip++];
    const uint8_t struct_size_length = core->program.bytecode[core->registers.ip++];
    size_t struct_size = 0; /* we do not increment offset here*/
    memcpy(&struct_size, &core->program.bytecode[core->registers.ip],  struct_size_length);
    core->registers.ip += struct_size_length;

    TypeV_Class* c = (TypeV_Class*)core->registers.regs[17].ptr;
    // allocate memory for struct
    size_t mem = core_alloc_interface(core, fields_count, c);
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
    // fp - 8byte (for pointer) is the location of the pushed ip
    memcpy(core->stack.stack+core->registers.fp-8, &core->registers.ip, sizeof(uint64_t));

    // jump to the address
    core->registers.ip = adr;
}

void debug_regs(TypeV_Core* core){
    // read register index
    uint8_t i = core->program.bytecode[core->registers.ip++];
    ASSERT(i < MAX_REG, "Invalid register index");

    struct table t;
    table_init(&t,
               "R?", "R%d",
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

    table_add(&t, i,
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
    table_print(&t, 400, stdout);
    table_free(&t);
}

void halt(TypeV_Core* core) {
    exit(0);
}


