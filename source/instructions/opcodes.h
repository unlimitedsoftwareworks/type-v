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
 * I[b]: Immediate where b is the number of bytes (1, 2, 4, 8)
 * S: byte-size, 1, 2, 4, 8.
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
     * OP_MV_REG_MEM dest: R, src: R
     */
    OP_MV_REG_REG_PTR,

    /**
     * OP_MV_REG_NULL dest: R
     * sets reg R to null
     */
    OP_MV_REG_NULL,

    /**
     * OP_MV_REG_I: dest: R, value-size: Z, value: I
     */
    OP_MV_REG_I,

    /**
     * OP_MV_REG_CONST dest: R, offset-size: Z, offset: I, byteSize: S
     */
    OP_MV_REG_CONST,
    OP_MV_REG_CONST_PTR,


    /**
     * OP_MV_GLOBAL_REG_[size] offset-size: Z, offset: I, source: R, byteSize: S
     * moves S bytes from register R to global pool address offset
     */
    OP_MV_GLOBAL_REG,
    OP_MV_GLOBAL_REG_PTR,

    /**
     * OP_MV_REG_GLOBAL_[size] dest: reg, offset-size: Z, offset: I
     * moves S bytes from global pool address offset to register R
     */
    OP_MV_REG_GLOBAL,
    OP_MV_REG_GLOBAL_PTR,

    /**
     * OP_S_ALLOC dest: R, fields-count: I, struct-size: I (2bytes)
     * Creates new struct of given total ﬁelds count (arg1) and total memory
     * (arg2 and arg3), stores the address of the new struct into dest.
     */
    OP_S_ALLOC,

    /**
     * OP_S_ALLOC_SHADOW Dest: R, copy: R fields-count: I
     * Creates a shadow copy of a struct (who's address is stored in copy),
     * a shadow copy is a copy that points to the same data but with different
     * offset table. Copy address is stored given register
     */
    OP_S_ALLOC_SHADOW,

    /**
     * OP_S_SET_OFFSET dest: R, fieldIndex: I, offset-value: I (2 bytes)
     * Sets the offset value of field I, of the struct stored in dest
     * to the given offset value
     */
    OP_S_SET_OFFSET,

    /**
     * OP_S_SET_OFFSET_SHADOW src: R, fieldIndexSrc: I, fieldIndexTarget: I
     * Sets the offset value of field index fieldIndexSrc, of the struct
     * stored in src to the offset value of field index fieldIndexTarget,
     * of the original struct referenced by the shadow copy
     * ie. shadow_copy.offsets[fieldIndexSrc] = original.offsets[fieldIndexTarget]
     */
    OP_S_SET_OFFSET_SHADOW,

    /**
     * OP_S_LOADF_[size] dest: R, src: R fieldIndex: I, byteSize: S
     * Loads size bytes from field I of struct stored at src to register dest
     */
    OP_S_LOADF,
    OP_S_LOADF_PTR,

    /**
     * OP_S_STOREF_CONST_[size] dest: R, fieldIndex: I, constant-offset: I (8 bytes), byteSize: S
     * Stores [size] bytes from constant pool address offset to field I of
     * struct stored at dest
     */
    OP_S_STOREF_CONST,
    OP_S_STOREF_CONST_PTR,

    /**
     * OP_S_STOREF_REG_[size] dest: R, fieldIndex: I, source: R, byteSize: S
     * Stores [size] bytes from register R to field I of struct stored at dest
     */
    OP_S_STOREF_REG,
    OP_S_STOREF_REG_PTR,

    /**
     * OP_C_ALLOC dest:
     * R num-methods: I, class-fields-size: I (2 bytes), classId-size: Z, classId: I
     * Allocates new class of given total ﬁelds count (arg1) and total fields
     * size of (arg2 and arg3), stores the address of the new class into dest.
     */
    OP_C_ALLOC,

    /**
     * OP_C_STOREM destReg: R, methodIndex: I, methodAddress: I(8 bytes)
     * Stores methodAddress into method table index methodIndex of class stored in destReg
     */
    OP_C_STOREM,

    /**
     * OP_C_LOADM dest: R, classReg: R methodIndex: I
     * Loads method address from method table of class stored in classReg to register R
     */
    OP_C_LOADM,

    /**
     * OP_CSTOREF_REG_[size] classReg: R, fieldOffset: I (2 bytes), R: source register, byteSize: S
     * Stores [size] bytes from register R to field I of class stored at classReg
     */
    OP_C_STOREF_REG,
    OP_C_STOREF_REG_PTR,

    /**
     * OP_C_STOREF_CONST_[size] classReg: R, fieldOffset: I (2 bytes), offset: I (8 bytes), byteSize: S
     */
    OP_C_STOREF_CONST,
    OP_C_STOREF_CONST_PTR,


    /**
     * OP_C_LOADF_[size] dest: R, classReg: R, fieldOffset: I (2 bytes), byteSize: S
     * Loads [size] bytes from field I of class stored at classReg to register R
     */
    OP_C_LOADF,
    OP_C_LOADF_PTR,


    /**
     * OP_I_ALLOC dest: R, num_methods: I, class: R
     * Allocates new interface method table of given total methods count (arg1),
     * interface is based on class stored in dest. Interface address is
     * stored in class
     */
    OP_I_ALLOC,

    /**
     * OP_I_ALLOC_I dest: R, num_methods: I, interface: R
     * Allocates new interface from another interface,
     * inheriting its parent class, and storing the new interface
     * address in dest
     */
    OP_I_ALLOC_I,

    /**
     * OP_I_SET_OFFSET dest: R, methodIndex: I, offset-value: I (2bytes)
     * Sets the offset value of method I, of the interface stored in dest
     */
    OP_I_SET_OFFSET,

    /**
     * OP_I_SET_OFFSET_I dest: R, methodIndexSrc: I, methodIndexTarget: I, src interface: R
     * Updates the offset value of method index methodIndexSrc, of the interface src in
     * to the offset value of method index methodIndexTarget, of the interface stored in dest
     */
    OP_I_SET_OFFSET_I,

    /**
     * OP_I_SET_OFFSET_M dest: R, methodID: I (8bytes), methodIndex: I (2 bytes) jumpFailure: I (8 bytes)
     * find the method methodID from the base class of the interface stored in dest,
     * sets its offset in the current interface's methodIndex to the given offset value. If the methodID
     * is not found, it jumps to the given address
     */
    OP_I_SET_OFFSET_M,

    /**
     * OP_I_LOADM dest: R, src: R methodIndex: I
     * Loads method address from method table of interface stored in src to register dest
     */
    OP_I_LOADM,

    /**
     * OP_C_IS_C dest: R, src: R, classId: I (8 bytes)
     * Checks if the given interface who's stored in src class id is the
     * same as the given id. Stores the result in R
     */
    OP_I_IS_C,

    /**
     * OP_I_IS_I method_id: I (8 bytes), src: R, jump-address: I (8 bytes)
     * Checks if the base class of the interface which is stored in src has
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
     * OP_A_ALLOC dest: R, num_elements: I (8 bytes), element_size: Z
     * allocate array of num_elements of size element_size
     * stores the address of the array in dest
     */
    OP_A_ALLOC,

    /**
     * OP_A_EXTEND array: R, num_elements-size: Z, num_elements: I
     * extends the array stored in R by num_elements
     */
    OP_A_EXTEND,

    /**
     * OP_A_LEN dest: R, array: R
     * stores the length of the array stored in array in dest
     */
    OP_A_LEN,

    /**
     * OP_A_SLICE dest: R, array: R, start: R, end: R
     */
    OP_A_SLICE,

    /**
     * OP_A_STOREF_REG_[size] dest: R, index: R, source: R, byteSize: S
     * Stores [size] bytes from register src to field
     * index of array dest
     */
    OP_A_STOREF_REG,
    OP_A_STOREF_REG_PTR,

    /**
     * OP_A_STOREF_CONST_[size] dest: R, index: R, offset: I (8 bytes), byteSize: S
     * Stores [size] bytes from constant pool address offset to field
     * value stored in register index of array stored at dest
     */
    OP_A_STOREF_CONST,
    OP_A_STOREF_CONST_PTR,

    /**
     * OP_A_LOADF dest: R, index: R, src: R, byteSize: S
     * Loads [size] bytes from field value stored in register index
     * of array stored at src to register dest
     */
    OP_A_LOADF,
    OP_A_LOADF_PTR,


    /**
     * OP_PUSH register: R, bytes: S
     */
    OP_PUSH,
    OP_PUSH_PTR,

    /**
     * OP_PUSH_CONST offset-size: Z, offset: I, bytes: S
     */
    OP_PUSH_CONST,

    /**
     * OP_POP register: R, bytes: S
     */
    OP_POP,
    OP_POP_PTR,

    /**
     * OP_FN_ALLOC
     * Allocates a function state, which is the .next of the current active one
     */
    OP_FN_ALLOC,

    /**
     * OP_FN_SET_REG_[size] dest: R, source: R, byteSize: S
     * sets the value of the dest register in the .next function state to
     * the value of source register in the active function state
     */
    OP_FN_SET_REG,
    OP_FN_SET_REG_PTR,



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


    /**
     * OP_FN_RET returns the current function state (.prev).
     */
    OP_FN_RET,

    /**
     * OP_FN_GET_RET_REG dest: R, src: R, size: b
     */
    OP_FN_GET_RET_REG,
    OP_FN_GET_RET_REG_PTR,

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
    /**
     * OP_J_CMP_[type] arg1, arg2, cmpType: I(1 byte), jump-address: I (8 bytes)
     * compares arg1 and arg2 using the given comparison type
     * 0: equal
     * 1: not equal
     * 2: greater than
     * 3: greater than or equal
     * 4: less than
     * 5: less than or equal
     * if the comparison is true, it jumps to the given address
     */
    OP_J_CMP_U8,
    OP_J_CMP_I8,
    OP_J_CMP_U16,
    OP_J_CMP_I16,
    OP_J_CMP_U32,
    OP_J_CMP_I32,
    OP_J_CMP_U64,
    OP_J_CMP_I64,
    OP_J_CMP_F32,
    OP_J_CMP_F64,
    OP_J_CMP_PTR,

    /**
     * OP_REG_FFI nnameconst-offset-size: I, nameconst-offset: I, ID: I (2 bytes),
     * registers an FFI of the given name at code offset and with the given ID
     */
    OP_REG_FFI,

    /**
     * OP_OPEN_FFI ffi-id: I (2 bytes)
     * Opens the FFI with the given ID
     */
    OP_OPEN_FFI,

    /**
     * OP_LD_FFI dest: R, ffi-id I (2 bytes), fn-id: I (1b)
     * Load an FFI method into dest register, from ffi-id and fn-id
     */
    OP_LD_FFI,

    /**
     * OP_CALL_FFI reg: R
     * calls a FFI stored in reg
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

    /**
     * OP_HALT code: I (4bytes)
     */
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

    /**
     * OP_SPILL_ALLOC size: I (2 bytes)
     */
    OP_SPILL_ALLOC,

    /**
     * OP_SPILL_REG: slot (2 bytes), source: R
     */
    OP_SPILL_REG,

    /**
     * OP_UNSPILL_ALLOC dest: R, slot: I (2 bytes)
     */
    OP_UNSPILL_REG,

    /**
     * OP_CLOSURE_ALLOC, dest:R, fn_address: R, env-size: I
     * Allocates a closure, setting its function pointer to the address in
     * function-address and preparing an environment of environment-size slots.
     */
    OP_CLOSURE_ALLOC,

    /**
     * OP_CAPTURE_VAR, closure: R, source-reg: R, env-slot: I
     * Copies the value from the specified register in the current function
     * state to the specified environment slot in the closure.
     */
    OP_CAPTURE_VAR,

    /**
     * OP_CLOSURE_CALL, closure: R
     * Similar to OP_FN_RET, but includes additional logic to properly
     * restore the previous function state's environment.
     */
    OP_CLOSURE_CALL,

    /**
     * OP_CLOSURE_RET
     * Similar to OP_FN_RET, but includes additional logic to properly restore
     * the previous function state's environment.
     */
    OP_CLOSURE_RET,

    /**
     * OP_SET_CLOSURE_ENV, closure: R
     * Used to set up the function state's environment pointer to the environment
     * of the specified closure.
     */

    OP_SET_CLOSURE_ENV,

    /**
     * OP_GET_CLOSURE_VAR, dest: R, closure: R, env-slot: I
     * Retrieves a value from the specified slot in the closure's
     * environment and stores it in the specified register.
     */
    OP_GET_CLOSURE_VAR,
}TypeV_OpCode;

#endif //TYPE_V_OPCODES_H
