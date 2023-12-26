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
     * OP_S_SETF_PTR fieldIndex: I
     * marks of the field at index I as a pointer
     * for the GC. This might be inefficient, and replace with
     * struct definitions in the future
     * (structs defined in bytecode)
     */
    //OP_S_MARKF_PTR,

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
     * OP_S_SET_OFFSET_SHADOW fieldIndexSrc: I, fieldIndexTarget: I
     * Sets the offset value of field index fieldIndexSrc, of the struct
     * stored in R16 to the offset value of field index fieldIndexTarget,
     * of the original struct referenced by the shadow copy
     * ie. shadow_copy.offsets[fieldIndexSrc] = original.offsets[fieldIndexTarget]
     */
    OP_S_SET_OFFSET_SHADOW,

    /**
     * OP_S_LOADF dest: R, fieldIndex: I, size: S
     * Loads
     * bytes from field I of struct stored at R16 to register R
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
     * OP_C_ALLOCF   num-methods: I, class-fields-size-size: Z, class-fields-size: I
     * Allocates new class of given total ﬁelds count (arg1) and total fields
     * size of (arg2 and arg3), stores the address of the new class into R17.
     */
    OP_C_ALLOC,

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
     * OP_CSTOREF_REG_[size] fieldIndex: I, R: source register
     * Stores [size] bytes from register R to field I of class stored at R17
     */
    OP_C_STOREF_REG_8,
    OP_C_STOREF_REG_16,
    OP_C_STOREF_REG_32,
    OP_C_STOREF_REG_64,
    OP_C_STOREF_REG_PTR,


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
     * OP_I_ALLOC_I num_methods: I, interface: R
     * Allocates new interface from another interface,
     * inheriting its parent class, and storing the new interface
     * address in R18
     */
    OP_I_ALLOC_I,

    /**
     * OP_I_SET_OFFSET methodIndex: I, offset-size: Z, offset-value: I
     * Sets the offset value of method I, of the interface stored in R18
     */
    OP_I_SET_OFFSET,

    /**
     * OP_I_SET_OFFSET_I methodIndexSrc: I, methodIndexTarget: I, src interface: R
     * Updates the offset value of method index methodIndexSrc, of the interface src in
     * to the offset value of method index methodIndexTarget, of the interface stored in R18
     */
    OP_I_SET_OFFSET_I,

    /**
     * OP_I_LOADM dest: R, methodIndex: I
     * Loads method address from method table of interface stored in R18 to register R
     */
    OP_I_LOADM,

    /**
     * OP_C_IS_C dest: R, classId: I (8 bytes)
     * Checks if the given interface who's stored in R18' class id is the
     * same as the given id. Stores the result in R
     */
    OP_I_IS_C,

    /**
     * OP_IS_I method_id: I (8 bytes), jump-address-offset: Z, jump-address: I
     * Checks if the base class of the interface which is stored in R18 has
     * a method with the same given ID. If a method with the same ID is found,
     * it continues. Otherwise, it jumps to the given address.
     */
    OP_I_IS_I,

    /**
     * OP_I_GET_C dest: R, interface: R
     * Gets the class of the given interface, stores the address of the class in R
     */
    OP_I_GET_C,

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
     * from type-c args are pushed in reverse order,
     * meaning bottom of the stack is the first argument
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
     * example: OP_CAST_I8_U8 R0
     *
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
     * OP_UPCAST_[I|U|F] dest: R, from: S, to: S
     * up casts the value in register R from given bytes
     * to target bytes. Overrides the value in R.
     * Example: OP_UPCAST_I R0, 4, 8
     * upcasts the value in R0 from 4 bytes to 8 bytes
     */
    OP_UPCAST_I,
    OP_UPCAST_U,
    OP_UPCAST_F,

    /**
     * OP_UPCAST_[I|U|F] dest: R, from: S, to: S
     * down casts the value in register R from given bytes
     * to target bytes. Overrides the value in R.
     * Example: OP_DOWNCAST_I R0, 8, 4
     * downcasts the value in R0 from 8 bytes to 4 bytes
     */
    OP_DCAST_I,
    OP_DCAST_U,
    OP_DCAST_F,

    /***
     * Math operations
     * OP_[math]_[type] dest: R, op1: R, op2: R
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
     * OP_P_DEQUEUE dest: R, promise: Rm
     * gets a message from the current process message queue,
     * stores the message in dest and the address of the promise
     * in the given register
     */
    OP_P_DEQUEUE,

    /**
     * OP_P_QUEUE_SIZE dest: R,
     */
    OP_P_QUEUE_SIZE,

    /**
     * OP_P_EMIT targetProcess: Rm, data: Rm, Promise: Rm
     * Emits data from the current process to the target
     * process message queue. Rm stores the address of the
     * allocated promise
     */
    OP_P_EMIT,


    /**
     * OP_P_WAIT_QUEUE,
     * Waits for the current process queue to receive a message
     */
    OP_P_WAIT_QUEUE,


    /**
     * OP_P_SEND_SEG targetProcess: Rm, data: I (<255)
     * Sends a signal to the target process,
     * Signals include KILL, PAUSE, RESUME, etc.
     */
    OP_P_SEND_SIG,

    /**
     * OP_P_ID dest: R
     */
    /**
     * OP_P_ID dest: R, process: Pm
     * Returns process id into dest reg R[u32] of process Pm
     */
    OP_P_ID,

    /**
     * Returns current process ID
     * OP_P_CID dest: R
     */
    OP_P_CID,

    /**
     * OP_P_STATUS dest: R, process: Pm
     * Returns process status into dest reg R[u8] of process Pm
     */
    OP_P_STATE,


    /**
     * OP_PROMISE_ALLOC dest: Rm
     * Allocates a new promise, stores its address in dest
     */
    OP_PROMISE_ALLOC,

    /**
     * OP_PROMISE_RESOLVE promise: Rm, payload: Rm
     * Resolves the given promise with the given payload
     */
    OP_PROMISE_RESOLVE,

    /**
     * OP_PROMISE_AWAIT promise: Rm
     * Awaits the given promise
     */
    OP_PROMISE_AWAIT,

    /**
     * OP_PROMISE_DATA dest: R, promise: Rm,
     * Returns promise data into dest reg R of promise Pm
     * Promise must have been resolved, otherwise fails
     */
    OP_PROMISE_DATA,

    /**
     * OP_LOCK_ALLOC dest: Rm, data: Rm
     * Allocates a new lock, containing data, stores its address in dest
     */
    OP_LOCK_ALLOC,

    /**
     * OP_LOCK_ACQUIRE lock: Rm, data: R,
     * Acquires the given lock. Will block if the lock is already acquired.
     * i.e waiting for lock promise to resolve. Stores the lock data in
     * the given argument
     */
    OP_LOCK_ACQUIRE,

    /**
     * OP_LOCK_RELEASE lock: Rm
     * Releases the given lock.
     */
    OP_LOCK_RELEASE,


    OP_DEBUG_REG,
    OP_HALT,

    /**
     * OP_LOAD_STD libId: I, fnId: I, dest: R
     * Loads a standard library function into dest reg R
     */
    OP_LOAD_STD,

    /**
     * OP_VM_HEALTH dest: R
     * Returns VM health into dest reg R[u8]
     */
    OP_VM_HEALTH,
}TypeV_OpCode;

#endif //TYPE_V_OPCODES_H
