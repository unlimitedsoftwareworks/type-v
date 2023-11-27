//
// Created by praisethemoon on 21.11.23.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "instructions.h"
#include "../core.h"
#include "../utils/utils.h"

#define PTR_SIZE sizeof(void*)

#define MAX_REG 18

void mv_reg_reg(TypeV_Core* core){
    const uint8_t target = core->program.bytecode[core->registers.ip++];
    const uint8_t source = core->program.bytecode[core->registers.ip++];
    const uint8_t bytesize = core->program.bytecode[core->registers.ip++];

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

mv_reg_const(8, 1)
mv_reg_const(16, 2)
mv_reg_const(32, 4)
mv_reg_const(64, 8)
mv_reg_const(ptr, PTR_SIZE)
#undef mv_reg_const

void mv_reg_mem(TypeV_Core* core){
    const uint8_t target = core->program.bytecode[core->registers.ip++];
    const uint8_t source = core->program.bytecode[core->registers.ip++];
    const uint8_t bytesize = core->program.bytecode[core->registers.ip++];

    ASSERT(target < MAX_REG, "Invalid register index");
    ASSERT(source < MAX_REG, "Invalid register index");
    ASSERT(bytesize <= 8, "Invalid bit size");
    memcpy(&core->registers.regs[target], &core->stack.stack[core->registers.regs[source].u64], bytesize);
}

void mv_mem_reg(TypeV_Core* core){
    const uint8_t target = core->program.bytecode[core->registers.ip++];
    const uint8_t source = core->program.bytecode[core->registers.ip++];
    const uint8_t bytesize = core->program.bytecode[core->registers.ip++];

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
    ASSERT(offset < core->stack.limit, "Invalid stack offset");\
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
    ASSERT(offset < core->stack.limit, "Invalid stack offset");\
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

void debug_regs(TypeV_Core* core){
    // print all registers, for each register print its value as a hex number
    for(int i = 0; i < MAX_REG; i++){
        // print each register i8, u8, i16, u16, i32, u32, i64, u64, f32, f64, ptr and hex values
        printf("R%d: i8: %d, u8: %u, i16: %d, u16: %u, i32: %d, u32: %u, i64: %lld, u64: %llu, f32: %f, f64: %lf, ptr: %p, hex: %llx\n",
               i,
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
    }
}

void halt(TypeV_Core* core) {
    exit(0);
}


