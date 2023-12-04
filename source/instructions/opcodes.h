/**
 * Type-V Virtual Machine
 * Author: praisethemoon
 * opcodes.h: VM Instructions Enums
 * VM instructions are defined here.
 */

#ifndef TYPE_V_OPCODES_H
#define TYPE_V_OPCODES_H



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
     * OP_MV_REG_I: dest: R, value-size: Z, value: I
     */
    OP_MV_REG_I,

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
     * OP_MV_REG_LOCAL_[size] dest: R, offset-size: Z, offset: I
     * moves S bytes from local stack address frame-pointer + offset to register R
     */
    OP_MV_REG_LOCAL_8,
    OP_MV_REG_LOCAL_16,
    OP_MV_REG_LOCAL_32,
    OP_MV_REG_LOCAL_64,
    OP_MV_REG_LOCAL_PTR,

    /**
     * OP_MV_LOCAL_REG offset-size: Z, offset: I, src: R
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
     * OP_S_LOADF dest: R, fieldIndex: I, size: S
     * Loads S bytes from field I of struct stored at R16 to register R
     */
    OP_S_LOADF,

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
     * OP_CSTOREF fieldIndex: I, R: source register, size: S
     * Stores [size] bytes from register R to field I of class stored at R17
     */
    OP_C_STOREF_REG,

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
     * OP_C_LOADF_[size] dest: R, fieldIndex: I, size: S
     * Loads [size] bytes from field I of class stored at R17 to register R
     */
    OP_C_LOADF,

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
     * OP_A_ALLOC num_elements_size: Z, num_elements: I, element_size: Z
     * allocate array of num_elements of size element_size
     * stores the address of the array in R19
     */
    OP_A_ALLOC,

    /**
     * OP_A_EXTEND num_elements_size: Z, num_elements: I
     * extends the array stored in R19 by num_elements
     */
    OP_A_EXTEND,

    /**
     * OP_A_STOREF_REG index: R, source: R, size: S
     * Stores [size] bytes from register R to field
     * value stored in register index of array stored at R19
     */
    OP_A_STOREF_REG,

    /**
     * OP_A_STOREF_CONST_[size] index: R, offset-size : Z, offset-: I
     * Stores [size] bytes from constant pool address offset to field
     * value stored in register index of array stored at R19
     */
    OP_A_STOREF_CONST_8,
    OP_A_STOREF_CONST_16,
    OP_A_STOREF_CONST_32,
    OP_A_STOREF_CONST_64,
    OP_A_STOREF_CONST_PTR,

    /**
     * OP_A_LOADF dest: R, index: R, size: S
     * Loads [size] bytes from field value stored in register index
     * of array stored at R19 to register R
     */
    OP_A_LOADF,

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
     *  OP_FRAME_INIT_LOCALS size-length: Z, size: I
     *  Extends the previous stack frame with the given size.
     *  The given size is the total size of local variables.
     *  Variables are pushed to the stack in the order they
     *  are declared.
     */
    OP_FRAME_INIT_LOCALS,

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
     * after the function exits. R20 is not popped,
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
    /**
     * OP_FN_CALLI function-address-size: Z, function-address: I
     * Calls the function at the given address
     * This function is called after the stack frame is initialized
     * and the arguments are pushed to the stack
     */
    OP_FN_CALLI,

    /** Casting instructions*/

    /**
     * OP_CAST_[d1]_[d2] dest: R
     * casts the value in register R from the given type
     * to the given type. Overrides the value in R.
     * OP_CAST is used to cast between types of the same
     * size
     */
    OP_CAST_I8_U8,
    OP_CAST_U8_I8,
    OP_CAST_I16_U16,
    OP_CAST_U16_I16,
    OP_CAST_I32_U32,
    OP_CAST_U32_I32,
    OP_CAST_I64_U64,
    OP_CAST_U64_I64,

    OP_CAST_I32_F32,
    OP_CAST_F32_I32,
    OP_CAST_I64_F64,
    OP_CAST_F64_I64,

    /**
     * OP_UPCAST_[d1]_[d2] dest: R
     * casts the value in register R from the given type
     * to the given type. Overrides the value in R.
     */
    OP_UPCAST_I8_I16,
    OP_UPCAST_U8_U16,
    OP_UPCAST_I16_I32,
    OP_UPCAST_U16_U32,
    OP_UPCAST_I32_I64,
    OP_UPCAST_U32_U64,
    OP_UPCAST_F32_F64,

    /**
     * OP_DCAST_[d1]_[d2] dest: R
     * down casts the value in register R from the given type
     * to the given type. Overrides the value in R.
     */
    OP_DCAST_I16_I8,
    OP_DCAST_U16_U8,
    OP_DCAST_I32_I16,
    OP_DCAST_U32_U16,
    OP_DCAST_I64_I32,
    OP_DCAST_U64_U32,
    OP_DCAST_F64_F32,

    /***
     * Math operations
     * OP_[math]_[type] op1: R, op2: R, dest: R
     * performs the given math operation on op1 and op2
     * and stores the result in dest
     */
    OP_ADD_I8,
    OP_ADD_U8,
    OP_ADD_I16,
    OP_ADD_U16,
    OP_ADD_I32,
    OP_ADD_U32,
    OP_ADD_I64,
    OP_ADD_U64,
    OP_ADD_F32,
    OP_ADD_F64,

    OP_ADD_PTR_U8,
    OP_ADD_PTR_U16,
    OP_ADD_PTR_U32,
    OP_ADD_PTR_U64,

    OP_SUB_I8,
    OP_SUB_U8,
    OP_SUB_I16,
    OP_SUB_U16,
    OP_SUB_I32,
    OP_SUB_U32,
    OP_SUB_I64,
    OP_SUB_U64,
    OP_SUB_F32,
    OP_SUB_F64,

    OP_SUB_PTR_U8,
    OP_SUB_PTR_U16,
    OP_SUB_PTR_U32,
    OP_SUB_PTR_U64,

    OP_MUL_I8,
    OP_MUL_U8,
    OP_MUL_I16,
    OP_MUL_U16,
    OP_MUL_I32,
    OP_MUL_U32,
    OP_MUL_I64,
    OP_MUL_U64,
    OP_MUL_F32,
    OP_MUL_F64,

    OP_DIV_I8,
    OP_DIV_U8,
    OP_DIV_I16,
    OP_DIV_U16,
    OP_DIV_I32,
    OP_DIV_U32,
    OP_DIV_I64,
    OP_DIV_U64,
    OP_DIV_F32,
    OP_DIV_F64,

    OP_MOD_I8,
    OP_MOD_U8,
    OP_MOD_I16,
    OP_MOD_U16,
    OP_MOD_I32,
    OP_MOD_U32,
    OP_MOD_I64,
    OP_MOD_U64,

    OP_LSHIFT_I8,
    OP_LSHIFT_U8,
    OP_LSHIFT_I16,
    OP_LSHIFT_U16,
    OP_LSHIFT_I32,
    OP_LSHIFT_U32,
    OP_LSHIFT_I64,
    OP_LSHIFT_U64,

    OP_RSHIFT_I8,
    OP_RSHIFT_U8,
    OP_RSHIFT_I16,
    OP_RSHIFT_U16,
    OP_RSHIFT_I32,
    OP_RSHIFT_U32,
    OP_RSHIFT_I64,
    OP_RSHIFT_U64,

    OP_CMP_I8,
    OP_CMP_U8,
    OP_CMP_I16,
    OP_CMP_U16,
    OP_CMP_I32,
    OP_CMP_U32,
    OP_CMP_I64,
    OP_CMP_U64,
    OP_CMP_F32,
    OP_CMP_F64,
    OP_CMP_PTR,

    OP_BAND_8,
    OP_BAND_16,
    OP_BAND_32,
    OP_BAND_64,

    OP_BOR_8,
    OP_BOR_16,
    OP_BOR_32,
    OP_BOR_64,

    OP_BXOR_8,
    OP_BXOR_16,
    OP_BXOR_32,
    OP_BXOR_64,

    OP_BNOT_8,
    OP_BNOT_16,
    OP_BNOT_32,
    OP_BNOT_64,

    OP_AND,
    OP_OR,
    OP_NOT,

    OP_J,
    OP_JE,
    OP_JNE,
    OP_JG,
    OP_JGE,
    OP_JL,
    OP_JLE,

    /**
     * OP_LD_FFI dest: R, const-offset-size: Z, const-offset: I
     * Load a FFI shared object library, stores
     * its memory address into R, the name is
     * stored in the constant pool at the given offset
     */
    OP_LD_FFI,

    /**
     * OP_CALL_FFI FFI-address: R, function-name-offset-size: Z, function-name-offset: I
     * calls a FFI based on its index
     */
    OP_CALL_FFI,

    OP_CLOSE_FFI,

    /**
     * OP_P_ALLOC dest: R, initfn-offset-size: Z, initfn-offset: I
     * Allocates a new process, storing its address in R
     * and calls the init function at the given offset
     */
    OP_P_ALLOC,

    /**
     * OP_P_SPAWN process-address: R
     * Spawns a new process from the given process address
     */
    //OP_P_SPAWN,

    /**
     * OP_P_DEQUEUE dest: R
     * gets a message from the current process message queue
     */
    OP_P_DEQUEUE,

    /**
     * OP_P_QUEUE_SIZE dest: R,
     */
    OP_P_QUEUE_SIZE,

    /**
     * OP_P_EMIT targetProcess: Rm, data: Rm
     * Emits data from the current process to the target
     * process message queue
     */
    //OP_P_EMIT,

    /**
     * OP_P_RET
     * Indicates that the current process has finished
     * processing a message and ready to process the next
     */
    //OP_P_RELOOP,

    /**
     * OP_P_SEND_SEG targetProcess: Rm, data: I (<255)
     * Sends a signal to the target process,
     * Signals include KILL, PAUSE, RESUME, etc.
     */
    //OP_P_SEND_SIG,

    /**
     * OP_P_ID dest: R
     */
    //OP_P_ID,
    //OP_P_STATUS,
    //OP_P_HEATH,



    /*
    OP_PROMISE_ALLOC,
    OP_PROMISE_RESOLVE,
    OP_PROMISE_REJECT,
    OP_PROMISE_AWAIT,

    OP_LOCK_ALLOC,
    OP_LOCK_ACQUIRE,
    OP_LOCK_RELEASE,
     */


    OP_DEBUG_REG,
    OP_HALT,
}TypeV_OpCode;

#endif //TYPE_V_OPCODES_H
