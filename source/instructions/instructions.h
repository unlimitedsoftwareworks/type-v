/**
 * Instructions for Type-V VM
 * Author: praisethemoon
 * instructions.h: VM Instructions
 * VM instructions are defined here. File is long. Yes.
 */
#ifndef TYPE_V_INSTRUCTIONS_H
#define TYPE_V_INSTRUCTIONS_H

#include "../core.h"

/**
 * Terminology:
 * R: Register
 * Rm: Register containing memory address (read as ptr)
 * I: Immediate
 * Z: Immediate value size in bytes (for immediate values > 255 when applicable)
 * C: Constant (offset)
 * S: byte-size, 1, 2, 3, 4 or ptr (size of pointer)
 */
typedef enum TypeV_OpCode {
    /**
     * OP_MOV_REG_REG dest: R, src: R, bits: S
     * Move 1byte from src to dest
     */
    OP_MV_REG_REG = 0x00,

    /**
     * OP_MV_REG_CONST_[size] dest: R, offset-size: Z, offset: I
     * OP_MV_REG_CONST_8 R0, 0x1, 0x10
     */
    OP_MV_REG_CONST_8,
    OP_MV_REG_CONST_16,
    OP_MV_REG_CONST_32,
    OP_MV_REG_CONST_64,
    OP_MV_REG_CONST_PTR,

    /**
     * OP_MV_REG_MEM dest: R, src: Rm, bits: S
     */
    OP_MV_REG_MEM,

    /**
     * OP_MV_MEM_REG dest: Rm, src: R, bits: S
     */
    OP_MV_MEM_REG,

    /**
     * OP_MV_REG_LOCAL_[size] offset-size: Z, offset: I, source: R
     */
    OP_MV_REG_LOCAL_8,
    OP_MV_REG_LOCAL_16,
    OP_MV_REG_LOCAL_32,
    OP_MV_REG_LOCAL_64,
    OP_MV_REG_LOCAL_PTR,

    /**
     * OP_MV_LOCAL_REG offset-size, src: R, offset-size: Z, offset: I
     */
    OP_MV_LOCAL_REG_8,
    OP_MV_LOCAL_REG_16,
    OP_MV_LOCAL_REG_32,
    OP_MV_LOCAL_REG_64,
    OP_MV_LOCAL_REG_PTR,

    /**
     * OP_MV_GLOBAL_REG_[size] offset-size: Z, offset: I, source: R
     */
    OP_MV_GLOBAL_REG_8,
    OP_MV_GLOBAL_REG_16,
    OP_MV_GLOBAL_REG_32,
    OP_MV_GLOBAL_REG_64,
    OP_MV_GLOBAL_REG_PTR,

    /**
     * OP_MV_REG_GLOBAL_[size] dest: reg, offset-size: Z, offset: I
     */
    OP_MV_REG_GLOBAL_8,
    OP_MV_REG_GLOBAL_16,
    OP_MV_REG_GLOBAL_32,
    OP_MV_REG_GLOBAL_64,
    OP_MV_REG_GLOBAL_PTR,



    OP_DEBUG_REGS,
    OP_HALT,
}TypeV_OpCode;

#define VM_INSTRUCTION(instruction) void instruction(TypeV_Core *core);

VM_INSTRUCTION(mv_reg_reg)
VM_INSTRUCTION(mv_reg_const_8)
VM_INSTRUCTION(mv_reg_const_16)
VM_INSTRUCTION(mv_reg_const_32)
VM_INSTRUCTION(mv_reg_const_64)
VM_INSTRUCTION(mv_reg_const_ptr)
VM_INSTRUCTION(mv_reg_mem)
VM_INSTRUCTION(mv_mem_reg)

VM_INSTRUCTION(mv_reg_local_8)
VM_INSTRUCTION(mv_reg_local_16)
VM_INSTRUCTION(mv_reg_local_32)
VM_INSTRUCTION(mv_reg_local_64)
VM_INSTRUCTION(mv_reg_local_ptr)

VM_INSTRUCTION(mv_local_reg_8)
VM_INSTRUCTION(mv_local_reg_16)
VM_INSTRUCTION(mv_local_reg_32)
VM_INSTRUCTION(mv_local_reg_64)
VM_INSTRUCTION(mv_local_reg_ptr)

VM_INSTRUCTION(mv_global_reg_8)
VM_INSTRUCTION(mv_global_reg_16)
VM_INSTRUCTION(mv_global_reg_32)
VM_INSTRUCTION(mv_global_reg_64)
VM_INSTRUCTION(mv_global_reg_ptr)

VM_INSTRUCTION(mv_reg_global_8)
VM_INSTRUCTION(mv_reg_global_16)
VM_INSTRUCTION(mv_reg_global_32)
VM_INSTRUCTION(mv_reg_global_64)
VM_INSTRUCTION(mv_reg_global_ptr)

VM_INSTRUCTION(debug_regs)
VM_INSTRUCTION(halt)


typedef void (*op_func)(TypeV_Core *);
static op_func op_funcs[] = {
        &mv_reg_reg,

        &mv_reg_const_8,
        &mv_reg_const_16,
        &mv_reg_const_32,
        &mv_reg_const_64,
        &mv_reg_const_ptr,

        &mv_reg_mem,
        &mv_mem_reg,

        &mv_reg_local_8,
        &mv_reg_local_16,
        &mv_reg_local_32,
        &mv_reg_local_64,
        &mv_reg_local_ptr,

        &mv_local_reg_8,
        &mv_local_reg_16,
        &mv_local_reg_32,
        &mv_local_reg_64,
        &mv_local_reg_ptr,

        &mv_global_reg_8,
        &mv_global_reg_16,
        &mv_global_reg_32,
        &mv_global_reg_64,
        &mv_global_reg_ptr,

        &mv_reg_global_8,
        &mv_reg_global_16,
        &mv_reg_global_32,
        &mv_reg_global_64,
        &mv_reg_global_ptr,

        &debug_regs,
        &halt,
};

#endif //TYPE_V_INSTRUCTIONS_H
