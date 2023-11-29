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
 * I: Immediate (when used alone, its <= 255), (when used with Z, it can be >= 255)
 * Z: Immediate value size in bytes (for immediate values > 255 when applicable)
 * C: Constant (offset)
 * S: byte-size, 1, 2, 4, 8 or 0 for pointer
 */
typedef enum TypeV_OpCode {
    /**
     * MOVE instruction,
     * format: MV_[dest]_[src]_[size]
     */

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
     * OP_MV_REG_MEM dest: R, src: Rm, bytes: S
     * moves S bytes from memory address in Rm to R
     */
    OP_MV_REG_MEM,

    /**
     * OP_MV_MEM_REG dest: Rm, src: R, bytes: S
     * moves S bytes from R to memory address in Rm
     */
    OP_MV_MEM_REG,

    /**
     * OP_MV_REG_LOCAL_[size] offset-size: Z, offset: I, source: R
     * moves S bytes from local stack address frame-pointer + offset to register R
     */
    OP_MV_REG_LOCAL_8,
    OP_MV_REG_LOCAL_16,
    OP_MV_REG_LOCAL_32,
    OP_MV_REG_LOCAL_64,
    OP_MV_REG_LOCAL_PTR,

    /**
     * OP_MV_LOCAL_REG offset-size, src: R, offset-size: Z, offset: I
     * moves S bytes from register R to local stack address frame-pointer + offset
     */
    OP_MV_LOCAL_REG_8,
    OP_MV_LOCAL_REG_16,
    OP_MV_LOCAL_REG_32,
    OP_MV_LOCAL_REG_64,
    OP_MV_LOCAL_REG_PTR,

    /**
     * OP_MV_GLOBAL_REG_[size] offset-size: Z, offset: I, source: R
     * moves S bytes from register R to global pool address offset
     */
    OP_MV_GLOBAL_REG_8,
    OP_MV_GLOBAL_REG_16,
    OP_MV_GLOBAL_REG_32,
    OP_MV_GLOBAL_REG_64,
    OP_MV_GLOBAL_REG_PTR,

    /**
     * OP_MV_REG_GLOBAL_[size] dest: reg, offset-size: Z, offset: I
     * moves S bytes from global pool address offset to register R
     */
    OP_MV_REG_GLOBAL_8,
    OP_MV_REG_GLOBAL_16,
    OP_MV_REG_GLOBAL_32,
    OP_MV_REG_GLOBAL_64,
    OP_MV_REG_GLOBAL_PTR,

    /**
     * OP_MV_REG_ARG_[size] dest: R, arg: offset-size: Z, offset: I
     * moves [size] bytes from argument stack address frame-pointer + offset to register R
     */
    OP_MV_REG_ARG_8,
    OP_MV_REG_ARG_16,
    OP_MV_REG_ARG_32,
    OP_MV_REG_ARG_64,
    OP_MV_REG_ARG_PTR,

    /**
     * OP_MV_ARG_REG_[size] arg: offset-size: Z, offset: I, source: R
     * moves [size] bytes from register R to argument stack address frame-pointer + offset
     */
    OP_MV_ARG_REG_8,
    OP_MV_ARG_REG_16,
    OP_MV_ARG_REG_32,
    OP_MV_ARG_REG_64,
    OP_MV_ARG_REG_PTR,

    /**
     * OP_S_ALLOC fieldOffsets-count: I, struct-size-size: Z, struct-size: I
     * Creates new struct of given total ﬁelds count (arg1) and total memory
     * (arg2 and arg3), stores the address of the new struct into R16.
     */
    OP_S_ALLOC,

    /**
     * OP_S_ALLOC_SHADOW fieldOffsets-count: I
     * Creates a shadow copy of a struct (who's address is stored in R16),
     * a shadow copy is a copy that points to the same data but with different
     * offset table. Copy address is stored in (and overrides) R16
     */
    OP_S_ALLOC_SHADOW,

    /**
     * OP_S_SET_OFFSET fieldIndex: I, offset-size: Z, offset-value: I
     * Sets the offset value of field I, of the struct stored in R16
     * to the given offset value
     */
    OP_S_SET_OFFSET,

    /**
     * OP_S_LOADF_[size] dest: R, fieldIndex: I
     * Loads [size] bytes from field I of struct stored at R16 to register R
     */
    OP_S_LOADF_8,
    OP_S_LOADF_16,
    OP_S_LOADF_32,
    OP_S_LOADF_64,
    OP_S_LOADF_PTR,

    /**
     * OP_S_STOREF_CONST_[size] fieldIndex: I, offset-size : Z, offset-: I
     * Stores [size] bytes from constant pool address offset to field I of
     * struct stored at R16
     */
    OP_S_STOREF_CONST_8,
    OP_S_STOREF_CONST_16,
    OP_S_STOREF_CONST_32,
    OP_S_STOREF_CONST_64,
    OP_S_STOREF_CONST_PTR,

    /**
     * OP_S_STOREF_REG fieldIndex: I, source: R, size: S
     * Stores [size] bytes from register R to field I of struct stored at R16
     */
    OP_S_STOREF_REG,

    /**
     * OP_C_ALLOCF fields-count: I, class-fields-size-size: Z, class-fields-size: I
     * Allocates new class of given total ﬁelds count (arg1) and total fields
     * size of (arg2 and arg3), stores the address of the new class into R17.
     */
    OP_C_ALLOCF,

    /**
     * OP_C_ALLOCM num_methods: I
     * Allocates new class method table of given total methods count (arg1),
     * Class address must be stored in R17
     */
    OP_C_ALLOCM,

    /**
     * OP_C_STOREM methodIndex: I, methodAddress: I
     * Stores method address (arg2) into method table of class stored in R17
     */
    OP_C_STOREM,

    /**
     * OP_C_LOADM dest: R, methodIndex: I
     * Loads method address from method table of class stored in R17 to register R
     */
    OP_C_LOADM,

    /**
     * OP_CSTOREF_[size] fieldIndex: I, R: source register, size: S
     * Stores [size] bytes from register R to field I of class stored at R17
     */
    OP_C_STOREF_8,
    OP_C_STOREF_16,
    OP_C_STOREF_32,
    OP_C_STOREF_64,
    OP_C_STOREF_PTR,

    /**
     * OP_C_STOREF_CONST_[size] fieldIndex: I, offset-size : Z, offset-: I
     * Stores [size] bytes from constant pool address offset to field I of
     * class stored at R17
     */
    OP_C_STOREF_CONST_8,
    OP_C_STOREF_CONST_16,
    OP_C_STOREF_CONST_32,
    OP_C_STOREF_CONST_64,
    OP_C_STOREF_CONST_PTR,

    /**
     * OP_C_LOADF_[size] dest: R, fieldIndex: I
     * Loads [size] bytes from field I of class stored at R17 to register R
     */
    OP_C_LOADF_8,
    OP_C_LOADF_16,
    OP_C_LOADF_32,
    OP_C_LOADF_64,
    OP_C_LOADF_PTR,

    /**
     * OP_I_ALLOC num_methods: I
     * Allocates new interface method table of given total methods count (arg1),
     * interface is based on class stored in R17. Interface address is
     * stored in R18
     */
    OP_I_ALLOC,

    /**
     * OP_I_SET_OFFSET methodIndex: I, offset-size: Z, offset-value: I
     * Sets the offset value of method I, of the interface stored in R18
     */
    OP_I_SET_OFFSET,

    /**
     * OP_I_LOADM dest: R, methodIndex: I
     * Loads method address from method table of interface stored in R18 to register R
     */
    OP_I_LOADM,

    /**
     * OP_PUSH register: R, bytes: S
     */
    OP_PUSH,

    /**
     * OP_PUSH_CONST offset-size: Z, offset: I, bytes: S
     */
    OP_PUSH_CONST,

    /**
     * OP_POP register: R, bytes: S
     */
    OP_POP,

    /**
     * OP_FRAME_INIT_ARGS size-length: Z, size: I
     * Creates a stack-frame of the given size.
     * The stack frame is initialized with the specified size
     * that is already stored in the constant pool.
     */
    OP_FRAME_INIT_ARGS,

    /**
     *  OP_FRAME_INIT_LOCAL size-length: Z, size: I
     *  Extends the previous stack frame with the given size.
     *  The given size is the total size of local variables.
     *  Variables are pushed to the stack in the order they
     *  are declared.
     */
    OP_FRAME_INIT_LOCAL,

    /**
     * OP_FRAME_RM removes the current stack frame from the stack
     * everything on top of the frame will be removed
     */
    OP_FRAME_RM,

    /**
     * OP_FRAME_PRECALL
     * pushes core state into the stack
     * to be popped after the function exits
     */
    OP_FRAME_PRECALL,

    /**
     * OP_FN_MAIN
     * Make sure that the stack contains the arguments
     * and local variables allocated,
     * i.e stack pointer equals frame_end
     */
    OP_FN_MAIN,

    /**
     * OP_FN_RET
     * pops core state from the stack
     * after the function exits. R19 is not popped,
     * it is used to store the return value of the function
     * Returns the IP to its previous value, hence, exits
     * the function
     */
    OP_FN_RET,

    /**
     * OP_FN_CALL function-address: R
     * Calls the function at the address stored in the given register
     * This function is called after the stack frame is initialized
     * and the arguments are pushed to the stack
     */
    OP_FN_CALL,


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

VM_INSTRUCTION(mv_reg_arg_8)
VM_INSTRUCTION(mv_reg_arg_16)
VM_INSTRUCTION(mv_reg_arg_32)
VM_INSTRUCTION(mv_reg_arg_64)
VM_INSTRUCTION(mv_reg_arg_ptr)

VM_INSTRUCTION(mv_arg_reg_8)
VM_INSTRUCTION(mv_arg_reg_16)
VM_INSTRUCTION(mv_arg_reg_32)
VM_INSTRUCTION(mv_arg_reg_64)
VM_INSTRUCTION(mv_arg_reg_ptr)

VM_INSTRUCTION(s_alloc)
VM_INSTRUCTION(s_alloc_shadow)
VM_INSTRUCTION(s_set_offset)

VM_INSTRUCTION(s_loadf_8)
VM_INSTRUCTION(s_loadf_16)
VM_INSTRUCTION(s_loadf_32)
VM_INSTRUCTION(s_loadf_64)
VM_INSTRUCTION(s_loadf_ptr)

VM_INSTRUCTION(s_storef_const_8)
VM_INSTRUCTION(s_storef_const_16)
VM_INSTRUCTION(s_storef_const_32)
VM_INSTRUCTION(s_storef_const_64)
VM_INSTRUCTION(s_storef_const_ptr)

VM_INSTRUCTION(s_storef_reg)

VM_INSTRUCTION(c_allocf)
VM_INSTRUCTION(c_allocm)
VM_INSTRUCTION(c_storem)
VM_INSTRUCTION(c_loadm)

VM_INSTRUCTION(c_storef_8)
VM_INSTRUCTION(c_storef_16)
VM_INSTRUCTION(c_storef_32)
VM_INSTRUCTION(c_storef_64)
VM_INSTRUCTION(c_storef_ptr)

VM_INSTRUCTION(c_storef_const_8)
VM_INSTRUCTION(c_storef_const_16)
VM_INSTRUCTION(c_storef_const_32)
VM_INSTRUCTION(c_storef_const_64)
VM_INSTRUCTION(c_storef_const_ptr)

VM_INSTRUCTION(c_loadf_8)
VM_INSTRUCTION(c_loadf_16)
VM_INSTRUCTION(c_loadf_32)
VM_INSTRUCTION(c_loadf_64)
VM_INSTRUCTION(c_loadf_ptr)

VM_INSTRUCTION(i_alloc)
VM_INSTRUCTION(i_set_offset)
VM_INSTRUCTION(i_loadm)

VM_INSTRUCTION(push)
VM_INSTRUCTION(push_const)
VM_INSTRUCTION(pop)

VM_INSTRUCTION(frame_init_args)
VM_INSTRUCTION(frame_init_locals)
VM_INSTRUCTION(frame_rm)
VM_INSTRUCTION(frame_precall)

VM_INSTRUCTION(fn_main)
VM_INSTRUCTION(fn_ret)
VM_INSTRUCTION(fn_call)


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

        &mv_reg_arg_8,
        &mv_reg_arg_16,
        &mv_reg_arg_32,
        &mv_reg_arg_64,
        &mv_reg_arg_ptr,

        &mv_arg_reg_8,
        &mv_arg_reg_16,
        &mv_arg_reg_32,
        &mv_arg_reg_64,
        &mv_arg_reg_ptr,

        &s_alloc,
        &s_alloc_shadow,
        &s_set_offset,

        &s_loadf_8,
        &s_loadf_16,
        &s_loadf_32,
        &s_loadf_64,
        &s_loadf_ptr,

        &s_storef_const_8,
        &s_storef_const_16,
        &s_storef_const_32,
        &s_storef_const_64,
        &s_storef_const_ptr,

        &s_storef_reg,

        &c_allocf,
        &c_allocm,

        &c_storem,
        &c_loadm,
        &c_storef_8,
        &c_storef_16,
        &c_storef_32,
        &c_storef_64,
        &c_storef_ptr,

        &c_storef_const_8,
        &c_storef_const_16,
        &c_storef_const_32,
        &c_storef_const_64,
        &c_storef_const_ptr,

        &c_loadf_8,
        &c_loadf_16,
        &c_loadf_32,
        &c_loadf_64,
        &c_loadf_ptr,

        &i_alloc,
        &i_set_offset,
        &i_loadm,

        &push,
        &push_const,
        &pop,

        &frame_init_args,
        &frame_init_locals,
        &frame_rm,
        &frame_precall,
        &fn_main,
        &fn_ret,
        &fn_call,

        &debug_regs,
        &halt,
};

#endif //TYPE_V_INSTRUCTIONS_H
